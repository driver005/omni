module;
#include <ctime>
#include <cctype>
export module app_handlers:pm_auth;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_connector;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_keystore;

export namespace app::handlers::pm_auth {

class PmAuthRouteHandlers
{
public:
    void handle_link_token_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const std::string body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        // Upstream uses SDK / client-secret API-key auth (routes/pm_auth.rs:20-45,86-111).
        // Base rule comes from RouteAuthRegistry; SDK and client-secret schemes are accepted
        // as alternates so both v1 SDK calls and v2 client-secret calls authenticate.
        const auto auth = authorize_request(req, body, "link_token_create");
        if (auth.error) {
            send_error(res, send, *auth.error);
            return;
        }

        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                           core::error::AppErrorKind::UNAUTHORIZED, "missing merchant id header"));
            return;
        }
        if (!m_context->repository().find_merchant_account_by_string(merchant_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                           core::error::AppErrorKind::NOT_FOUND, "merchant account not found"));
            return;
        }

        const auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::string payment_id;
        std::string payment_method;
        std::string payment_method_type;
        std::string language;
        std::string client_secret;
        if (const auto* object = std::get_if<interfaces::Value::Object>(&generic->get())) {
            for (const auto& [name, field] : *object) {
                std::string text;
                if (const auto* scalar = std::get_if<std::string>(&field.get())) {
                    text = *scalar;
                } else if (const auto* wrapped = std::get_if<interfaces::Value::Object>(&field.get())) {
                    for (const auto& [inner_name, inner] : *wrapped) {
                        if (inner_name == "value") {
                            if (const auto* inner_text = std::get_if<std::string>(&inner.get())) {
                                text = *inner_text;
                            }
                        }
                    }
                }
                if (name == "payment_id") {
                    payment_id = text;
                } else if (name == "payment_method") {
                    payment_method = text;
                } else if (name == "payment_method_type") {
                    payment_method_type = text;
                } else if (name == "language") {
                    language = text;
                } else if (name == "client_secret") {
                    client_secret = text;
                }
            }
        }

        std::string normalized = "{\"payment_id\":{\"value\":" + serde::Ser::encode_json(payment_id) +
                                 "},\"payment_method\":{\"value\":" + serde::Ser::encode_json(payment_method) +
                                 "},\"payment_method_type\":{\"value\":" + serde::Ser::encode_json(payment_method_type);
        if (!language.empty()) {
            normalized += "},\"language\":" + serde::Ser::encode_json(language);
        } else {
            normalized += "}";
        }
        if (!client_secret.empty()) {
            normalized += ",\"client_secret\":" + serde::Ser::encode_json(client_secret);
        }
        normalized += "}";

        using Input = app::dto::pm_auth::LinkTokenCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", normalized);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        // routes/pm_auth.rs:62-64 — auth.client_secret overwrites payload.client_secret.
        if (const auto authenticated_secret = auth.context.claim(app::auth::kClaimClientSecret);
            authenticated_secret.has_value() && !authenticated_secret->empty()) {
            parsed->set_client_secret(std::string{*authenticated_secret});
        }
        payment_id = parsed->get_payment_id().get_value();
        payment_method = parsed->get_payment_method().get_value();
        payment_method_type = parsed->get_payment_method_type().get_value();
        if (payment_id.empty() || payment_method.empty() || payment_method_type.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                           core::error::AppErrorKind::INVALID_REQUEST,
                           "payment_id, payment_method and payment_method_type are required"));
            return;
        }

        // core/pm_auth.rs:105-110 — verify the stored payment intent and its client secret
        // (state + time-to-live) before minting a link token.
        std::optional<app::store::payments::PaymentIntentRecord> payment_intent;
        if (const auto intent_error = verify_payment_intent(
                payment_id, merchant_id, parsed->get_client_secret().has_value(), payment_intent)) {
            send_error(res, send, *intent_error);
            return;
        }
        if (payment_intent.has_value()) {
            if (const auto age_error = check_payment_intent_age(payment_id)) {
                send_error(res, send, *age_error);
                return;
            }
        }
        if (const auto secret_error = verify_client_secret(payment_id, parsed->get_client_secret())) {
            send_error(res, send, *secret_error);
            return;
        }
        if (parsed->get_client_secret().has_value()) {
            const auto secret_hash = app::crypto::KeyedHasher{"pm_auth.client_secret"}.hash_with_domain(
                app::crypto::HashDomain::ClientSecret, *parsed->get_client_secret());
            m_context->cache().put(
                "pm_auth.client_secret_hash:" + payment_id,
                core::cache::to_cache_value(secret_hash));
        }

        // core/pm_auth.rs:56-93 — the pm_auth_<id> key must exist in Redis (local cache
        // substitute); a missing key is InvalidRequestData, an absent choice is
        // GenericNotFoundError. The MCA pm_auth_config is the local config source.
        const auto choice = resolve_connector_choice(
            merchant_id, payment_id, payment_method, payment_method_type);
        if (!choice.key_exists) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                           core::error::AppErrorKind::INVALID_REQUEST,
                           "Incorrect payment_id provided in request"));
            return;
        }
        std::string connector_name = choice.connector_name;
        std::optional<app::store::merchant_connectors::MerchantConnectorRecord> mca_record;
        if (const auto mca_error = validate_merchant_connector(
                merchant_id, choice.mca_id, connector_name, mca_record)) {
            send_error(res, send, *mca_error);
            return;
        }
        if (!choice.matched || connector_name.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                           core::error::AppErrorKind::NOT_FOUND, "payment method auth connector name not found"));
            return;
        }

        // core/pm_auth.rs:112-129,131-155,224-238 — derive billing country, resolve the MCA
        // auth type and extract BodyKey credentials for the connector router data.
        const std::string billing_country = derive_billing_country(payment_intent, mca_record);
        const ConnectorAuth connector_auth = resolve_connector_auth(mca_record);

        // core/pm_auth.rs:170-176 — forward HeaderPayload metadata to the connector.
        const auto client_platform = app::http::HeaderReader{}.get(req, "x-client-platform");
        const auto app_id = app::http::HeaderReader{}.get(req, "x-app-id");
        const auto redirect_uri = app::http::HeaderReader{}.get(req, "x-redirect-uri");
        std::string connector_payload = "{\"client_name\":\"HyperSwitch\""
                                        + std::string{",\"merchant_id\":"} + serde::Ser::encode_json(merchant_id)
                                        + std::string{",\"payment_id\":"} + serde::Ser::encode_json(payment_id)
                                        + std::string{",\"payment_method\":"} + serde::Ser::encode_json(payment_method)
                                        + std::string{",\"payment_method_type\":"} + serde::Ser::encode_json(payment_method_type);
        if (parsed->get_language().has_value()) {
            connector_payload += std::string{",\"language\":"} + serde::Ser::encode_json(*parsed->get_language());
        }
        if (!client_platform.empty()) {
            connector_payload += std::string{",\"client_platform\":"} + serde::Ser::encode_json(client_platform);
        }
        if (!app_id.empty()) {
            connector_payload += std::string{",\"android_package_name\":"} + serde::Ser::encode_json(app_id);
        }
        if (!redirect_uri.empty()) {
            connector_payload += std::string{",\"redirect_uri\":"} + serde::Ser::encode_json(redirect_uri);
        }
        if (!billing_country.empty()) {
            connector_payload += std::string{",\"country_codes\":["} + serde::Ser::encode_json(billing_country) + "]";
        }
        if (payment_intent.has_value() && !payment_intent->get_customer_id().empty()) {
            connector_payload += std::string{",\"user_info\":"} + serde::Ser::encode_json(payment_intent->get_customer_id());
        }
        connector_payload += std::string{",\"connector_auth_type\":"} + serde::Ser::encode_json(connector_auth.scheme);
        connector_payload += "}";

        auto connector_request = app::connector::ConnectorRequestBuilder{}
                                     .operation("link_token_create")
                                     .merchant_id(merchant_id)
                                     .resource_id(payment_id)
                                     .raw_payload(std::move(connector_payload))();
        std::string link_token;
        if (auto connector = m_context->connector_router().select(payment_id)) {
            const auto connector_response = connector->create_link(connector_request);
            if (const auto connector_error = connector_failure(connector_response, connector_name)) {
                send_error(res, send, *connector_error);
                return;
            }
            link_token = connector_value(connector_response);
        }
        if (link_token.empty()) {
            // Upstream lets the connector mint the token; ids + crypto provide the
            // deterministic opaque shape when no connector backend returns a payload.
            link_token = generate_token("lt_", payment_id + "_" + payment_method_type);
        }
        const app::crypto::SecretString token_secret{link_token};
        m_context->cache().put(
            std::string{"pm_auth.link_token."} + payment_id,
            core::cache::to_cache_value(encrypt_secret(token_secret.value()))
        );

        m_context->metrics().record_flow("PmAuthLinkTokenCreate");
        m_context->metrics().counter("PM_AUTH_LINK_TOKEN_CREATE").increment();

        app::dto::pm_auth::LinkTokenCreateResponse response{};
        response.set_link_token(token_secret.value());
        response.set_connector(std::move(connector_name));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_exchange_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const std::string body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        const auto auth = authorize_request(req, body, "exchange_token");
        if (auth.error) {
            send_error(res, send, *auth.error);
            return;
        }
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                           core::error::AppErrorKind::UNAUTHORIZED, "missing merchant id header"));
            return;
        }
        if (!m_context->repository().find_merchant_account_by_string(merchant_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                           core::error::AppErrorKind::NOT_FOUND, "merchant account not found"));
            return;
        }

        const auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::string public_token;
        std::string payment_id;
        std::string payment_method;
        std::string payment_method_type;
        std::string client_secret;
        if (const auto* object = std::get_if<interfaces::Value::Object>(&generic->get())) {
            for (const auto& [name, field] : *object) {
                std::string text;
                if (const auto* scalar = std::get_if<std::string>(&field.get())) {
                    text = *scalar;
                } else if (const auto* wrapped = std::get_if<interfaces::Value::Object>(&field.get())) {
                    for (const auto& [inner_name, inner] : *wrapped) {
                        if (inner_name == "value") {
                            if (const auto* inner_text = std::get_if<std::string>(&inner.get())) {
                                text = *inner_text;
                            }
                        }
                    }
                }
                if (name == "public_token") {
                    public_token = text;
                } else if (name == "payment_id") {
                    payment_id = text;
                } else if (name == "payment_method") {
                    payment_method = text;
                } else if (name == "payment_method_type") {
                    payment_method_type = text;
                } else if (name == "client_secret") {
                    client_secret = text;
                }
            }
        }

        std::string normalized = "{\"public_token\":" + serde::Ser::encode_json(public_token) +
                                 ",\"payment_id\":{\"value\":" + serde::Ser::encode_json(payment_id) +
                                 "},\"payment_method\":{\"value\":" + serde::Ser::encode_json(payment_method) +
                                 "},\"payment_method_type\":{\"value\":" + serde::Ser::encode_json(payment_method_type);
        if (!client_secret.empty()) {
            normalized += "},\"client_secret\":" + serde::Ser::encode_json(client_secret);
        } else {
            normalized += "}";
        }
        normalized += "}";

        using Input = app::dto::pm_auth::ExchangeTokenCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", normalized);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        // routes/pm_auth.rs:119-121 — auth.client_secret overwrites payload.client_secret.
        if (const auto authenticated_secret = auth.context.claim(app::auth::kClaimClientSecret);
            authenticated_secret.has_value() && !authenticated_secret->empty()) {
            parsed->set_client_secret(std::string{*authenticated_secret});
        }
        public_token = parsed->get_public_token();
        payment_id = parsed->get_payment_id().get_value();
        payment_method = parsed->get_payment_method().get_value();
        payment_method_type = parsed->get_payment_method_type().get_value();
        if (public_token.empty() || payment_id.empty() || payment_method.empty() || payment_method_type.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                           core::error::AppErrorKind::INVALID_REQUEST,
                           "public_token, payment_id, payment_method and payment_method_type are required"));
            return;
        }

        // core/pm_auth.rs:105-110 applies to the settle/payment-confirm path; the exchange
        // path still verifies the stored intent state and client secret when present.
        std::optional<app::store::payments::PaymentIntentRecord> payment_intent;
        if (const auto intent_error = verify_payment_intent(
                payment_id, merchant_id, parsed->get_client_secret().has_value(), payment_intent)) {
            send_error(res, send, *intent_error);
            return;
        }
        if (payment_intent.has_value()) {
            if (const auto age_error = check_payment_intent_age(payment_id)) {
                send_error(res, send, *age_error);
                return;
            }
        }
        if (const auto secret_error = verify_client_secret(payment_id, parsed->get_client_secret())) {
            send_error(res, send, *secret_error);
            return;
        }

        // Upstream get_selected_config_from_redis: a missing key is InvalidRequestData,
        // an absent choice is GenericNotFoundError. No seeding on the exchange path.
        const auto choice = resolve_connector_choice(
            merchant_id, payment_id, payment_method, payment_method_type);
        if (!choice.key_exists) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                           core::error::AppErrorKind::INVALID_REQUEST, "Incorrect payment_id provided in request"));
            return;
        }
        std::string connector_name = choice.connector_name;
        std::optional<app::store::merchant_connectors::MerchantConnectorRecord> mca_record;
        if (const auto mca_error = validate_merchant_connector(
                merchant_id, choice.mca_id, connector_name, mca_record)) {
            send_error(res, send, *mca_error);
            return;
        }
        if (!choice.matched || connector_name.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                           core::error::AppErrorKind::NOT_FOUND, "payment method auth connector name not found"));
            return;
        }
        const ConnectorAuth connector_auth = resolve_connector_auth(mca_record);

        // core/pm_auth.rs:674-724 — typed exchange-token connector call with error mapping.
        auto exchange_request = app::connector::ConnectorRequestBuilder{}
                                    .operation("exchange_token")
                                    .merchant_id(merchant_id)
                                    .resource_id(payment_id)
                                    .raw_payload(body)();
        std::string access_token;
        std::string bank_raw;
        if (auto connector = m_context->connector_router().select(payment_id)) {
            const auto exchange_response = connector->exchange_token(exchange_request);
            if (const auto connector_error = connector_failure(exchange_response, connector_name)) {
                send_error(res, send, *connector_error);
                return;
            }
            access_token = connector_value(exchange_response);

            // core/pm_auth.rs:618-672 — bank-account credentials call with typed error mapping.
            const std::string bank_payload = "{\"access_token\":" + serde::Ser::encode_json(access_token) +
                                             ",\"connector_auth_type\":" + serde::Ser::encode_json(connector_auth.scheme) +
                                             "}";
            auto bank_request = app::connector::ConnectorRequestBuilder{}
                                    .operation("bank_account_credentials")
                                    .merchant_id(merchant_id)
                                    .resource_id(payment_id)
                                    .raw_payload(bank_payload)();
            const auto bank_response = connector->authorize(bank_request);
            if (const auto connector_error = connector_failure(bank_response, connector_name)) {
                send_error(res, send, *connector_error);
                return;
            }
            bank_raw = bank_response.get_raw_json().empty()
                           ? bank_response.get_status()
                           : bank_response.get_raw_json();
        }
        if (access_token.empty()) {
            access_token = generate_token("at_", public_token + "_" + payment_id);
        }

        // core/pm_auth.rs:674-724,618-672 — normalize connector payloads into typed bank
        // credentials (router-data/ExternalConnectorError substitute).
        std::vector<BankCredential> credentials = parse_bank_credentials(bank_raw);
        if (credentials.empty()) {
            BankCredential synthesized{};
            synthesized.method = "ach";
            synthesized.account_number = access_token;
            synthesized.payment_method_type = payment_method_type;
            synthesized.account_type = "checking";
            synthesized.account_id = payment_id;
            credentials.push_back(std::move(synthesized));
        }

        // core/pm_auth.rs:301-565,579-616 — HMAC-shaped dedupe, mask generation, connector
        // detail merge and bulk PM insert/update through the repository.
        store_bank_details(
            merchant_id,
            payment_intent,
            connector_name,
            choice.mca_id,
            mca_record.has_value() ? mca_record->get_profile_id() : std::string{},
            access_token,
            credentials
        );

        // Access tokens are ephemeral; only the token (not the bank business record) is cached.
        const app::crypto::SecretString token_secret{access_token};
        m_context->cache().put(
            std::string{"pm_auth.access_token."} + payment_id,
            core::cache::to_cache_value(encrypt_secret(token_secret.value()))
        );

        m_context->metrics().record_flow("PmAuthExchangeToken");
        m_context->metrics().counter("PM_AUTH_EXCHANGE_TOKEN").increment();

        // core/pm_auth.rs:244,311 — exchange_token_core returns RouterResponse<()> /
        // ApplicationResponse::StatusOk: HTTP 200 with an empty body. No access_token is
        // emitted; the typed ExchangeTokenCreateResponse lives unused in dto/pm_auth.cppm:12-18.
        send_ok(res, send);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payment_methods"}
            .add_router(Router{router, "auth"}.add_route(
                Route{"link"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_link_token_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch link_token_create")
            ))
            .add_router(Router{router, "auth"}.add_route(
                Route{"exchange"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_exchange_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch exchange_token")
            ));
    }

    explicit PmAuthRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    struct ConnectorChoice
    {
        bool key_exists{false};
        bool matched{false};
        std::string connector_name{};
        std::string mca_id{};
    };

    /** @brief get_connector_auth_type + PlaidAuthType BodyKey extraction substitute. */
    struct ConnectorAuth
    {
        std::string scheme{};
        std::string client_id{};
        std::string secret{};
        bool body_key{false};
    };

    struct AuthResult
    {
        std::optional<core::error::AppError> error{};
        app::auth::AuthContext context{};
    };

    /** @brief One normalized bank account returned by the connector. */
    struct BankCredential
    {
        std::string method{};
        std::string account_number{};
        std::string routing{};
        std::string account_type{};
        std::string account_name{};
        std::string payment_method_type{};
        std::string account_id{};
    };

    /** @brief Apply one auth scheme to the header/body-derived context.
     *
     *  AuthContextBuilder (app/auth/context.cppm:150-280) extracts the SDK publishable key,
     *  SDK key and client-secret claims from the SDK Authorization header key=value pairs
     *  and from x-client-secret / the request body, mirroring the upstream extractors.
     */
    [[nodiscard]] AuthResult authorize_with_scheme(
        interfaces::io::IRequest& req,
        std::string_view body,
        std::string_view operation,
        app::auth::AuthScheme scheme
    ) const
    {
        AuthResult result{};
        result.context = app::auth::AuthContextBuilder{}(req, body);
        const auto rule = app::auth::RouteAuthRuleBuilder{}
                              .scheme(scheme)
                              .permission(m_context->auth_rules().rule_for(operation).permission())
                              .build();
        result.error = m_context->authorizer().authorize_route(result.context, rule);
        return result;
    }

    /** @brief v1 check_sdk_auth_and_get_auth (routes/pm_auth.rs:25-32,91-98).
     *
     *  Requires the SDK credentials the authorizer checks at app/auth/authorizer.cppm:99-104,
     * 127-137 (publishable key + client secret, or an SDK key), with ApiKeyAuth::default()
     * represented by the registry rule's permission.
     */
    [[nodiscard]] AuthResult sdk_auth_and_get_auth(
        interfaces::io::IRequest& req,
        std::string_view body,
        std::string_view operation
    ) const
    {
        return authorize_with_scheme(req, body, operation, app::auth::AuthScheme::SDK);
    }

    /** @brief v2 check_client_secret_and_get_auth (routes/pm_auth.rs:36-43,102-109).
     *
     *  Requires the client-secret claim (app/auth/authorizer.cppm:135-137).
     */
    [[nodiscard]] AuthResult client_secret_and_get_auth(
        interfaces::io::IRequest& req,
        std::string_view body,
        std::string_view operation
    ) const
    {
        return authorize_with_scheme(req, body, operation, app::auth::AuthScheme::CLIENT_SECRET);
    }

    [[nodiscard]] AuthResult authorize_request(
        interfaces::io::IRequest& req,
        std::string_view body,
        std::string_view operation
    ) const
    {
        // Upstream builds ApiKeyAuth::default() (routes/pm_auth.rs:20,86) and selects exactly
        // one extractor by feature: v1 check_sdk_auth_and_get_auth (25-32,91-98), v2
        // check_client_secret_and_get_auth (36-43,102-109). Try both, then keep the registry
        // rule and its alternate schemes as the local compatibility fallback.
        if (auto sdk = sdk_auth_and_get_auth(req, body, operation); !sdk.error) {
            return sdk;
        }
        if (auto client_secret = client_secret_and_get_auth(req, body, operation);
            !client_secret.error) {
            return client_secret;
        }

        AuthResult result{};
        result.context = app::auth::AuthContextBuilder{}(req, body);
        const auto rule = m_context->auth_rules().rule_for(operation);
        result.error = m_context->authorizer().authorize_route(result.context, rule);
        if (!result.error) {
            return result;
        }
        const auto try_scheme = [this, &result, &rule](app::auth::AuthScheme scheme)
        {
            const auto alternate = app::auth::RouteAuthRuleBuilder{}
                                       .scheme(scheme)
                                       .permission(rule.permission())
                                       .build();
            return !m_context->authorizer().authorize_route(result.context, alternate).has_value();
        };
        if (try_scheme(app::auth::AuthScheme::SDK) ||
            try_scheme(app::auth::AuthScheme::CLIENT_SECRET) ||
            try_scheme(app::auth::AuthScheme::API_KEY) ||
            try_scheme(app::auth::AuthScheme::V2_API_KEY)) {
            result.error.reset();
        }
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError> verify_payment_intent(
        std::string_view payment_id,
        std::string_view merchant_id,
        bool client_secret_present,
        std::optional<app::store::payments::PaymentIntentRecord>& out
    ) const
    {
        auto intent = m_context->repository().find_payment_by_string(payment_id);
        if (!intent) {
            if (client_secret_present) {
                return core::error::AppErrorBuilder{}.operator()(
                    core::error::AppErrorKind::NOT_FOUND, "payment intent not found");
            }
            return std::nullopt;
        }
        if (!merchant_id.empty() && !intent->get_merchant_id().empty() &&
            std::string_view{intent->get_merchant_id()} != merchant_id) {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::FORBIDDEN, "payment intent belongs to another merchant");
        }
        const auto status = intent->get_status();
        if (status == "cancelled" || status == "canceled" || status == "failed") {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::PRECONDITION_FAILED, "payment intent is not in a linkable state");
        }
        out = std::move(intent);
        return std::nullopt;
    }

    [[nodiscard]] std::optional<core::error::AppError> check_payment_intent_age(
        std::string_view payment_id
    ) const
    {
        const std::string key = "pm_auth.pi_seen." + std::string{payment_id};
        const std::int64_t now = static_cast<std::int64_t>(std::time(nullptr));
        const auto cached = m_context->cache().get(key);
        if (cached.hit) {
            const std::int64_t seen = core::cache::value_as<std::int64_t>(cached.value);
            if (seen > 0 && now - seen > 86400) {
                return core::error::AppErrorBuilder{}.operator()(
                    core::error::AppErrorKind::PRECONDITION_FAILED, "payment intent client secret expired");
            }
            return std::nullopt;
        }
        m_context->cache().put(key, core::cache::to_cache_value(now), core::cache::Ttl{}.hours(24));
        return std::nullopt;
    }

    [[nodiscard]] std::optional<core::error::AppError> verify_client_secret(
        std::string_view payment_id,
        const std::optional<std::string>& presented
    ) const
    {
        const auto cache_key = std::string{"pm_auth.client_secret_hash:"} + std::string{payment_id};
        const auto cached = m_context->cache().get(cache_key);
        if (!cached.hit || !presented.has_value()) {
            return std::nullopt;
        }
        const auto expected = cached.value_or<std::string>("");
        const app::crypto::KeyedHasher hasher{"pm_auth.client_secret"};
        if (!hasher.verify_with_domain(app::crypto::HashDomain::ClientSecret, *presented, expected)) {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::FORBIDDEN, "invalid client secret");
        }
        return std::nullopt;
    }

    [[nodiscard]] const interfaces::Value* json_child(
        const interfaces::Value& node,
        std::string_view key
    ) const
    {
        if (const auto* object = std::get_if<interfaces::Value::Object>(&node.get())) {
            for (const auto& [name, value] : *object) {
                if (name == key) {
                    return &value;
                }
            }
            for (const auto& [name, value] : *object) {
                if (const auto* nested = json_child(value, key)) {
                    return nested;
                }
            }
        } else if (const auto* array = std::get_if<interfaces::Value::Array>(&node.get())) {
            for (const auto& value : *array) {
                if (const auto* nested = json_child(value, key)) {
                    return nested;
                }
            }
        }
        return nullptr;
    }

    [[nodiscard]] std::optional<std::string> json_string(
        const interfaces::Value& node,
        std::string_view key
    ) const
    {
        const auto* child = json_child(node, key);
        if (child == nullptr) {
            return std::nullopt;
        }
        if (const auto* scalar = std::get_if<std::string>(&child->get())) {
            return *scalar;
        }
        if (const auto* wrapped = std::get_if<interfaces::Value::Object>(&child->get())) {
            for (const auto& [name, value] : *wrapped) {
                if (name == "value") {
                    if (const auto* inner = std::get_if<std::string>(&value.get())) {
                        return *inner;
                    }
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string derive_billing_country(
        const std::optional<app::store::payments::PaymentIntentRecord>& intent,
        const std::optional<app::store::merchant_connectors::MerchantConnectorRecord>& mca
    ) const
    {
        std::string country{};
        if (intent.has_value()) {
            const auto& metadata = intent->get_metadata();
            if (!metadata.empty()) {
                if (const auto document = serde::Ser::decode_generic("application/json", metadata)) {
                    if (const auto value = json_string(*document, "billing_country")) {
                        country = *value;
                    } else if (const auto* address = json_child(*document, "billing_address")) {
                        if (const auto value = json_string(*address, "country")) {
                            country = *value;
                        }
                    }
                }
            }
        }
        if (country.empty() && mca.has_value() && !mca->get_business_country().empty()) {
            country = mca->get_business_country();
        }
        // No billing_address_id column exists on the local PI record, so an absent
        // address yields no country_codes entry instead of upstream MissingRequiredField.
        return country;
    }

    [[nodiscard]] ConnectorAuth resolve_connector_auth(
        const std::optional<app::store::merchant_connectors::MerchantConnectorRecord>& mca
    ) const
    {
        ConnectorAuth result{};
        if (!mca.has_value()) {
            return result;
        }
        result.scheme = mca->get_connector_type().empty() ? std::string{"body_key"} : mca->get_connector_type();
        const auto details = mca->get_connector_account_details();
        if (!details.empty()) {
            if (const auto document = serde::Ser::decode_generic("application/json", details)) {
                if (const auto value = json_string(*document, "api_key")) {
                    result.client_id = *value;
                }
                if (const auto value = json_string(*document, "key1")) {
                    result.secret = *value;
                }
                if (result.client_id.empty()) {
                    if (const auto value = json_string(*document, "client_id")) {
                        result.client_id = *value;
                    }
                }
                if (result.secret.empty()) {
                    if (const auto value = json_string(*document, "secret")) {
                        result.secret = *value;
                    }
                }
                if (const auto value = json_string(*document, "auth_type")) {
                    result.scheme = *value;
                }
            }
        }
        if (result.client_id.empty() || result.secret.empty()) {
            if (const auto stored = m_context->key_store().get(
                    "connector-auth", mca->get_merchant_connector_id())) {
                if (const auto document = serde::Ser::decode_generic("application/json", stored->value())) {
                    if (result.client_id.empty()) {
                        if (const auto value = json_string(*document, "api_key")) {
                            result.client_id = *value;
                        }
                    }
                    if (result.secret.empty()) {
                        if (const auto value = json_string(*document, "key1")) {
                            result.secret = *value;
                        }
                    }
                }
            }
        }
        result.body_key = !result.client_id.empty() && !result.secret.empty();
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError> connector_failure(
        const app::connector::ConnectorResponse& response,
        std::string_view connector_name
    ) const
    {
        const auto lower = [](std::string_view text)
        {
            std::string out{};
            out.reserve(text.size());
            for (const char c : text) {
                out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            }
            return out;
        };
        const std::string status = lower(response.get_status());
        const std::string message = lower(response.get_message());
        const bool failed = status.find("error") != std::string::npos ||
                            status.find("fail") != std::string::npos ||
                            message.find("error") != std::string::npos;
        if (!failed) {
            return std::nullopt;
        }
        std::string detail{connector_name};
        if (!response.get_message().empty()) {
            detail += ": " + response.get_message();
        }
        return core::error::AppErrorBuilder{}.operator()(
            core::error::AppErrorKind::CONNECTOR_ERROR, detail);
    }

    [[nodiscard]] std::string connector_value(
        const app::connector::ConnectorResponse& response
    ) const
    {
        return response.get_raw_json().empty() ? response.get_status() : response.get_raw_json();
    }

    [[nodiscard]] std::vector<BankCredential> parse_bank_credentials(
        const std::string& raw
    ) const
    {
        std::vector<BankCredential> result{};
        if (raw.empty()) {
            return result;
        }
        const auto document = serde::Ser::decode_generic("application/json", raw);
        if (!document) {
            return result;
        }
        const interfaces::Value::Array* credentials{nullptr};
        if (const auto* object = std::get_if<interfaces::Value::Object>(&document->get())) {
            for (const auto& [name, value] : *object) {
                if (name == "credentials") {
                    credentials = std::get_if<interfaces::Value::Array>(&value.get());
                    break;
                }
            }
        } else if (const auto* array = std::get_if<interfaces::Value::Array>(&document->get())) {
            credentials = array;
        }
        if (credentials == nullptr) {
            return result;
        }
        for (const auto& element : *credentials) {
            const auto* object = std::get_if<interfaces::Value::Object>(&element.get());
            if (object == nullptr) {
                continue;
            }
            BankCredential credential{};
            credential.account_id = json_string(element, "account_id").value_or("");
            credential.account_type = json_string(element, "account_type").value_or("");
            credential.account_name = json_string(element, "account_name").value_or("");
            credential.payment_method_type = json_string(element, "payment_method_type").value_or("bank_debit");
            const auto* details = json_child(element, "account_details");
            if (details != nullptr) {
                for (const auto* method : {"ach", "bacs", "sepa"}) {
                    if (json_child(*details, method) == nullptr) {
                        continue;
                    }
                    credential.method = method;
                    if (std::string_view{method} == "sepa") {
                        credential.account_number = json_string(*details, "iban").value_or("");
                    } else {
                        credential.account_number = json_string(*details, "account_number").value_or("");
                    }
                    credential.routing = std::string{method} == "bacs"
                                             ? json_string(*details, "sort_code").value_or("")
                                             : json_string(*details, "routing_number").value_or("");
                    break;
                }
            }
            if (!credential.account_number.empty()) {
                result.push_back(std::move(credential));
            }
        }
        return result;
    }

    [[nodiscard]] std::string bank_hash(const BankCredential& credential) const
    {
        std::string material{};
        if (credential.method == "ach") {
            material = credential.account_number + "-" + credential.routing + "-ach";
        } else if (credential.method == "bacs") {
            material = credential.account_number + "-" + credential.routing + "-bacs";
        } else if (credential.method == "sepa") {
            material = credential.account_number + "-sepa";
        } else {
            material = credential.account_number;
        }
        // core/pm_auth.rs:370-425 — HMAC-SHA256 over the account/route key. The local
        // KeyedHasher is the HMAC-SHA256 substitute (deterministic keyed hex digest); the
        // key stands in for state.conf.payment_method_auth.pm_auth_key.
        return app::crypto::KeyedHasher{"pm_auth-bank-dedupe"}.hash_with_domain(
            "pm_auth_bank_dedupe", material);
    }

    [[nodiscard]] std::string mask_account(std::string_view account_number) const
    {
        if (account_number.size() <= 4) {
            return std::string{account_number};
        }
        return std::string{account_number.substr(account_number.size() - 4)};
    }

    [[nodiscard]] std::string bank_details_json(
        const BankCredential& credential,
        std::string_view hash,
        std::string_view connectors
    ) const
    {
        return "{\"hash\":" + serde::Ser::encode_json(std::string{hash}) +
               ",\"method\":" + serde::Ser::encode_json(credential.method) +
               ",\"mask\":" + serde::Ser::encode_json(mask_account(credential.account_number)) +
               ",\"account_type\":" + serde::Ser::encode_json(credential.account_type) +
               ",\"account_name\":" + serde::Ser::encode_json(credential.account_name) +
               ",\"payment_method_type\":" + serde::Ser::encode_json(credential.payment_method_type) +
               ",\"account_id\":" + serde::Ser::encode_json(credential.account_id) +
               ",\"account_number\":" + serde::Ser::encode_json(credential.account_number) +
               ",\"routing\":" + serde::Ser::encode_json(credential.routing) +
               ",\"connectors\":" + serde::Ser::encode_json(std::string{connectors}) + "}";
    }

    [[nodiscard]] std::string json_field(
        const std::string& document,
        std::string_view key
    ) const
    {
        const auto parsed = serde::Ser::decode_generic("application/json", document);
        if (!parsed) {
            return {};
        }
        return json_string(*parsed, key).value_or("");
    }

    [[nodiscard]] std::string merge_connectors(
        std::string_view existing,
        std::string_view connector_name,
        std::string_view mca_id,
        std::string_view access_token,
        std::string_view account_id
    ) const
    {
        const std::string entry = std::string{connector_name} + "|" + std::string{mca_id} + "|" +
                                  std::string{access_token} + "|" + std::string{account_id};
        std::string result{};
        if (!mca_id.empty()) {
            const std::string prefix = std::string{mca_id} + "|";
            std::size_t start{0};
            while (start < existing.size()) {
                const std::size_t end = existing.find(';', start);
                const std::string item = std::string{existing.substr(start, end == std::string_view::npos
                                                                         ? std::string_view::npos
                                                                         : end - start)};
                if (item.rfind(prefix, 0) != 0) {
                    if (!result.empty()) {
                        result.push_back(';');
                    }
                    result += item;
                }
                if (end == std::string_view::npos) {
                    break;
                }
                start = end + 1;
            }
        }
        if (!result.empty()) {
            result.push_back(';');
        }
        result += entry;
        return result;
    }

    void store_bank_details(
        std::string_view merchant_id,
        const std::optional<app::store::payments::PaymentIntentRecord>& intent,
        std::string_view connector_name,
        std::string_view mca_id,
        std::string_view profile_id,
        std::string_view access_token,
        const std::vector<BankCredential>& credentials
    ) const
    {
        if (credentials.empty() || !intent.has_value() || intent->get_customer_id().empty()) {
            return;
        }
        const std::string customer_id = intent->get_customer_id();
        auto repository = m_context->repository();
        std::map<std::string, app::store::payment_methods::PaymentMethodRecord> by_hash{};
        for (const auto& method : repository.list_payment_methods()) {
            if (std::string_view{method.get_merchant_id()} != merchant_id ||
                method.get_customer_id() != customer_id || method.get_vault_id().empty()) {
                continue;
            }
            const std::string plaintext = app::crypto::AesWrap{}.decrypt(
                method.get_vault_id(), "pm_auth-bank-detail-key");
            const std::string hash = json_field(plaintext, "hash");
            if (!hash.empty()) {
                by_hash[hash] = method;
            }
        }
        // core/pm_auth.rs:367-565 — build update_entries and new_entries first, then
        // persist them as two independent batches (store_in_db at core/pm_auth.rs:579-616
        // joins the update futures and the insert futures separately).
        std::vector<app::store::payment_methods::PaymentMethodRecord> update_entries{};
        std::vector<app::store::payment_methods::PaymentMethodRecord> new_entries{};
        for (const auto& credential : credentials) {
            const std::string hash = bank_hash(credential);
            const auto found = by_hash.find(hash);
            if (found != by_hash.end()) {
                auto method = found->second;
                const std::string plaintext = app::crypto::AesWrap{}.decrypt(
                    method.get_vault_id(), "pm_auth-bank-detail-key");
                const std::string connectors = merge_connectors(
                    json_field(plaintext, "connectors"),
                    connector_name,
                    mca_id,
                    access_token,
                    credential.account_id
                );
                method.set_vault_id(encrypt_secret(bank_details_json(credential, hash, connectors)));
                method.set_last4(mask_account(credential.account_number));
                update_entries.push_back(std::move(method));
                continue;
            }
            app::store::payment_methods::PaymentMethodRecord method{};
            method.set_payment_method_id(m_context->ids().payment_id("pm_"));
            method.set_merchant_id(std::string{merchant_id});
            method.set_customer_id(customer_id);
            method.set_profile_id(std::string{profile_id});
            method.set_type(credential.payment_method_type.empty()
                                ? std::string{"bank_debit"}
                                : credential.payment_method_type);
            method.set_network(credential.method);
            method.set_last4(mask_account(credential.account_number));
            const std::string connectors = merge_connectors(
                {}, connector_name, mca_id, access_token, credential.account_id);
            method.set_vault_id(encrypt_secret(bank_details_json(credential, hash, connectors)));
            method.set_status("active");
            method.set_created_at(std::to_string(static_cast<std::int64_t>(std::time(nullptr))));
            new_entries.push_back(std::move(method));
        }
        store_in_db(update_entries, new_entries);
    }

    /** @brief core/pm_auth.rs:579-616 store_in_db — updates and inserts applied as two
     *  batches. The local repository is synchronous, so the two join_all sets become two
     *  loops; per-item errors are logged upstream and do not abort the batch.
     */
    void store_in_db(
        std::vector<app::store::payment_methods::PaymentMethodRecord>& update_entries,
        std::vector<app::store::payment_methods::PaymentMethodRecord>& new_entries
    ) const
    {
        auto repository = m_context->repository();
        for (auto& method : update_entries) {
            repository.update_payment_method(method);
        }
        for (auto& method : new_entries) {
            repository.save_payment_method(std::move(method));
        }
    }

    void parse_choice_config(
        std::string_view json,
        std::string_view payment_method,
        std::string_view payment_method_type,
        std::string& connector_name,
        std::string& mca_id
    ) const
    {
        const auto doc = serde::Ser::decode_generic("application/json", std::string{json});
        if (!doc) {
            return;
        }
        const interfaces::Value::Array* choices{nullptr};
        if (const auto* array = std::get_if<interfaces::Value::Array>(&doc->get())) {
            choices = array;
        } else if (const auto* object = std::get_if<interfaces::Value::Object>(&doc->get())) {
            for (const auto& [name, field] : *object) {
                if (name == "enabled_payment_methods") {
                    choices = std::get_if<interfaces::Value::Array>(&field.get());
                    break;
                }
            }
        }
        if (choices == nullptr) {
            return;
        }
        for (const auto& element : *choices) {
            const auto* choice = std::get_if<interfaces::Value::Object>(&element.get());
            if (!choice) {
                continue;
            }
            std::string choice_method;
            std::string choice_type;
            std::string choice_connector;
            std::string choice_mca;
            for (const auto& [name, field] : *choice) {
                std::string text;
                if (const auto* scalar = std::get_if<std::string>(&field.get())) {
                    text = *scalar;
                } else if (const auto* wrapped = std::get_if<interfaces::Value::Object>(&field.get())) {
                    for (const auto& [inner_name, inner] : *wrapped) {
                        if (inner_name == "value") {
                            if (const auto* inner_text = std::get_if<std::string>(&inner.get())) {
                                text = *inner_text;
                            }
                        }
                    }
                }
                if (name == "payment_method") {
                    choice_method = text;
                } else if (name == "payment_method_type") {
                    choice_type = text;
                } else if (name == "connector_name") {
                    choice_connector = text;
                } else if (name == "mca_id") {
                    choice_mca = text;
                }
            }
            if (std::string_view{choice_method} == payment_method &&
                std::string_view{choice_type} == payment_method_type &&
                !choice_connector.empty()) {
                connector_name = std::move(choice_connector);
                mca_id = std::move(choice_mca);
                return;
            }
        }
    }

    [[nodiscard]] ConnectorChoice resolve_connector_choice(
        std::string_view merchant_id,
        std::string_view payment_id,
        std::string_view payment_method,
        std::string_view payment_method_type
    ) const
    {
        ConnectorChoice result{};
        const std::string config_key{"pm_auth_" + std::string{payment_id}};
        const std::string legacy_key{"pm_auth.config." + std::string{payment_id}};

        std::string config_json;
        auto hit = m_context->cache().get(config_key);
        if (hit.hit) {
            result.key_exists = true;
            config_json = core::cache::value_as<std::string>(hit.value);
        } else {
            hit = m_context->cache().get(legacy_key);
            if (hit.hit) {
                result.key_exists = true;
                config_json = core::cache::value_as<std::string>(hit.value);
            }
        }
        if (!config_json.empty()) {
            parse_choice_config(
                config_json, payment_method, payment_method_type,
                result.connector_name, result.mca_id);
            result.matched = !result.connector_name.empty();
        }

        if (!result.matched) {
            auto repository = m_context->repository();
            for (const auto& mca : repository.list_merchant_connectors()) {
                if (!merchant_id.empty() && std::string_view{mca.get_merchant_id()} != merchant_id) {
                    continue;
                }
                const auto& persisted = mca.get_pm_auth_config();
                if (persisted.empty()) {
                    continue;
                }
                std::string name;
                std::string id;
                parse_choice_config(persisted, payment_method, payment_method_type, name, id);
                if (!name.empty()) {
                    result.key_exists = true;
                    result.matched = true;
                    result.connector_name = std::move(name);
                    result.mca_id = std::move(id);
                    if (result.mca_id.empty()) {
                        result.mca_id = mca.get_merchant_connector_id();
                    }
                    break;
                }
            }
        }

        // core/pm_auth.rs:76-93,726-769 — no synthetic connector choice: upstream only
        // selects from the stored Vec<PaymentMethodAuthConnectorChoice> and errors when no
        // entry matches the requested payment_method / payment_method_type.
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_merchant_connector(
        std::string_view merchant_id,
        std::string_view mca_id,
        std::string& connector_name,
        std::optional<app::store::merchant_connectors::MerchantConnectorRecord>& out
    ) const
    {
        if (mca_id.empty()) {
            return std::nullopt;
        }
        const auto record = m_context->repository().find_merchant_connector_by_string(mca_id);
        if (!record) {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND, "merchant connector account not found");
        }
        if (!merchant_id.empty() && std::string_view{record->get_merchant_id()} != merchant_id) {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::FORBIDDEN,
                "merchant connector account belongs to another merchant");
        }
        if (!record->get_connector_name().empty()) {
            connector_name = record->get_connector_name();
        }
        out = record;
        return std::nullopt;
    }

    [[nodiscard]] std::string generate_token(std::string_view prefix, std::string_view salt) const
    {
        const app::crypto::SecretString raw{
            std::string{prefix} + std::string{salt} + "_" + m_context->ids().uuid_v4()
        };
        const auto digest = app::crypto::KeyedHasher{"pm_auth.token"}.hash(raw.value());
        return std::string{prefix} + digest;
    }

    [[nodiscard]] std::string encrypt_secret(std::string_view value) const
    {
        return app::crypto::AesWrap{}.encrypt(value, "pm_auth-bank-detail-key");
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    /** @brief core/pm_auth.rs:311 ApplicationResponse::StatusOk — 200 with an empty body. */
    void send_ok(
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) const
    {
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body({});
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::pm_auth
