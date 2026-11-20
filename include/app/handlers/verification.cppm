module;
export module app_handlers:verification;

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
import app_services;

namespace app::handlers::verification {

/** @brief Local serializable request shape.
 *
 *  Upstream wire is `ApplepayMerchantVerificationRequest` with a plain string
 *  `merchant_connector_account_id` (a serde-transparent newtype). The converted
 *  app_dto carries a `MerchantConnectorAccountIdValue` wrapper with no serde
 *  support, so normalize to a vector of strings / plain string that serde can
 *  decode directly. A generic-tree fallback still accepts the wrapped form.
 */
class VerificationRequestWire
{
public:
    void set_domain_names(std::vector<std::string> value) { domain_names = std::move(value); }
    [[nodiscard]] const std::vector<std::string>& get_domain_names() const noexcept { return domain_names; }
    void set_merchant_connector_account_id(std::string value) { merchant_connector_account_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_connector_account_id() const noexcept { return merchant_connector_account_id; }
    std::vector<std::string> domain_names{};
    std::string merchant_connector_account_id{};
};

} // namespace app::handlers::verification

// Serialization declared before the handler bodies that use this shape.
template<>
struct serde::Serializable<::app::handlers::verification::VerificationRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "domain_names",
                &::app::handlers::verification::VerificationRequestWire::get_domain_names,
                &::app::handlers::verification::VerificationRequestWire::set_domain_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_account_id",
                &::app::handlers::verification::VerificationRequestWire::get_merchant_connector_account_id,
                &::app::handlers::verification::VerificationRequestWire::set_merchant_connector_account_id,
                serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "verification_request_wire"; }
};

export namespace app::handlers::verification {

class VerificationRouteHandlers
{
public:
    void handle_apple_pay_merchant_registration(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_verification(auth, "apple_pay_merchant_registration")) {
            send_error(res, send, *auth_error);
            return;
        }

        // Upstream route: POST /verify/apple_pay/{merchant_id} (v2: /v2/verify/apple-pay/{merchant_id}).
        // The merchant id is the last path segment on both the v1 and v2 routes.
        const auto segments = app::http::RequestPath{}(req);
        const std::string merchant_id = segments.count() > 0
            ? std::string{segments.at(segments.count() - 1).value_or(std::string_view{})}
            : std::string{};
        if (merchant_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "merchant id not found in path"));
            return;
        }

        // server_wrap checks the authenticated processor account against the path merchant id.
        const auto authenticated_merchant = authenticated_merchant_id(auth, req);
        if (!authenticated_merchant.empty() && authenticated_merchant != merchant_id) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "merchant id does not match authenticated account"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        // Typed decode of the upstream request wire; fall back to a generic tree
        // for the converted `{"value":...}` wrapper form.
        using Input = ::app::handlers::verification::VerificationRequestWire;
        std::vector<std::string> domain_names{};
        std::string merchant_connector_id{};
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (parsed) {
            domain_names = parsed->get_domain_names();
            merchant_connector_id = parsed->get_merchant_connector_account_id();
        } else {
            auto generic = serde::Ser::decode_generic("application/json", body);
            if (!generic) {
                send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            domain_names = parse_domain_names(*generic);
            merchant_connector_id = parse_merchant_connector_id(*generic);
        }
        if (domain_names.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::VALIDATION_FAILED, "no domain names supplied"));
            return;
        }
        if (merchant_connector_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "merchant connector account id missing"));
            return;
        }

        // get_apple_pay_merchant_connector_account: resolve the MCA for this merchant.
        auto repository = m_context->repository();
        auto connector_record = repository.find_merchant_connector_by_string(merchant_connector_id);
        if (!connector_record) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "merchant connector account not found"));
            return;
        }
        if (connector_record->get_merchant_id() != merchant_id) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "merchant connector account belongs to another merchant"));
            return;
        }

        // validate_profile_id_from_auth_layer (core/verification/utils.rs:52-57):
        // an authenticated profile id must match the MCA's stored profile id.
        if (const auto profile_error = validate_profile_id(auth.profile_id(), connector_record->get_profile_id())) {
            send_error(res, send, *profile_error);
            return;
        }

        // ApplepayMerchantVerificationConfigs is serialized camelCase for the Apple endpoint.
        // Upstream derives encrypt_to and partner_internal_merchant_identifier from
        // applepay_merchant_configs.common_merchant_identifier (core/verification.rs:28-40).
        const auto common_identifier = m_context->config_store()
                                           .get_value("applepay_merchant_configs", "common_merchant_identifier")
                                           .value_or(std::string{"Applepay_merchant"});
        app::dto::verifications::ApplepayMerchantVerificationConfigs configs{};
        configs.set_domain_names(domain_names);
        configs.set_encrypt_to(common_identifier);
        configs.set_partner_internal_merchant_identifier(common_identifier);
        configs.set_partner_merchant_name("Applepay_merchant");
        const auto verification_payload = serde::Ser::encode_json(configs);

        // Config-derived Apple endpoint and certificate references. The real
        // cert-signed HTTPS call needs the external Apple gateway plus PEM
        // material from a configured secret store; locally these feed only the
        // deterministic signature stand-in below (documented STUB in the report).
        const auto apple_endpoint = m_context->config_store()
                                        .get_value("applepay_merchant_configs", "applepay_endpoint")
                                        .value_or(std::string{"https://apple-pay-gateway.apple.com/paymentservices/registerMerchant"});
        const auto apple_cert = m_context->config_store()
                                    .get_value("applepay_merchant_configs", "applepay_certificate")
                                    .value_or(std::string{});
        const auto apple_cert_key = m_context->config_store()
                                        .get_value("applepay_merchant_configs", "applepay_certificate_key")
                                        .value_or(std::string{});

        // Merchant key store load via master key (get_merchant_key_store_by_merchant_id,
        // core/verification/utils.rs:27-50). Local substitute: resolve a configured
        // merchant key from KeyStore by (purpose, merchant id); fall back to the
        // persisted merchant account material. MCA records are already plaintext in
        // this port, so no runtime decryption step is available or required.
        std::string signing_seed{};
        if (const auto resolved = m_context->secret_resolver().resolve("merchant_key_store", merchant_id)) {
            signing_seed = *resolved;
        } else if (const auto key = m_context->key_store().get("merchant_key_store", merchant_id)) {
            signing_seed = key->value();
        }
        if (signing_seed.empty()) {
            if (const auto account = repository.find_merchant_account_by_string(merchant_id)) {
                if (!account->get_payment_response_hash_key().empty()) {
                    signing_seed = account->get_payment_response_hash_key();
                } else if (!account->get_publishable_key().empty()) {
                    signing_seed = account->get_publishable_key();
                }
            }
        }
        if (signing_seed.empty()) {
            signing_seed = connector_record->get_connector_account_details();
        }
        if (signing_seed.empty()) {
            signing_seed = "apple-pay-verification";
        }

        // Deterministic signature stand-in: digest the payload together with the
        // configured endpoint and keyed digests of the certificate material. No
        // secret value is ever placed in a cache key or a response.
        std::string signature_material{verification_payload};
        signature_material.append("|").append(apple_endpoint);
        signature_material.append("|").append(app::crypto::KeyedHasher{"applepay-cert"}.hash(apple_cert));
        signature_material.append("|").append(app::crypto::KeyedHasher{"applepay-cert-key"}.hash(apple_cert_key));
        app::crypto::SecretString merchant_key{signing_seed};
        const auto signature = app::crypto::KeyedHasher{merchant_key.value()}
                                   .hash_with_domain(app::crypto::HashDomain::WebhookSignature, signature_material);
        const auto verification_id = m_context->ids().uuid_v4();
        m_context->cache().put(
            "verification.apple_pay_request:" + verification_id,
            core::cache::to_cache_value(signature)
        );

        // call_connector_api stand-in: the connector result now gates persistence.
        // Upstream returns InvalidRequestData{"Applepay verification Failed"} on a
        // non-Ok response and only updates the MCA on success (core/verification.rs:65-92).
        auto connector = m_context->connector_router().select(merchant_id);
        bool connector_ok = false;
        if (connector) {
            app::connector::ConnectorRequest connector_request{};
            connector_request.set_operation("apple_pay_merchant_registration");
            connector_request.set_merchant_id(merchant_id);
            connector_request.set_resource_id(merchant_connector_id);
            connector_request.set_raw_payload(verification_payload);
            const auto connector_response = connector->authorize(connector_request);
            const auto& status = connector_response.get_status();
            connector_ok = !status.empty() && status != "failed" && status != "error" && status != "declined";
        }
        if (!connector_ok) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "Applepay verification Failed"));
            return;
        }

        // add_domain_to_apple_pay_merchant_connector_account: append only domains that
        // are not already verified, then persist the MCA through the repository.
        const auto existing = decode_domains(connector_record->get_applepay_verified_domains());
        const auto merged = merge_domains(existing, domain_names);
        connector_record->set_applepay_verified_domains(encode_domains(merged));
        if (!repository.update_merchant_connector(*connector_record)) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INTERNAL_ERROR, "failed to persist apple pay verified domains"));
            return;
        }

        // Upstream Flow::Verification instrumentation; LockAction::NotApplicable for
        // this route, so no lock is acquired. Metrics and the ephemeral cache marker
        // above are the local deterministic substitute for server_wrap tracing.
        m_context->metrics().record_flow("Verification");
        m_context->metrics().counter("VERIFICATION").increment();
        m_context->metrics().record_flow("ApplePayMerchantRegistration");
        m_context->metrics().counter("APPLE_PAY_MERCHANT_REGISTRATION").increment();

        app::dto::verifications::ApplepayMerchantResponse response{};
        response.set_status_message("Applepay verification Completed");
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_retrieve_apple_pay_verified_domains(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_verification(auth, "retrieve_apple_pay_verified_domains")) {
            send_error(res, send, *auth_error);
            return;
        }

        // Upstream GET reads merchant_id + merchant_connector_account_id from the query
        // string (ApplepayGetVerifiedDomainsParam). Both are required upstream, so a
        // missing field is INVALID_REQUEST rather than a merchant-wide scan.
        const auto query = app::http::RequestQuery{}(req);
        const auto merchant_param = query.get("merchant_id");
        const auto connector_param = query.get("merchant_connector_account_id");
        if (!merchant_param || merchant_param->empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "merchant_id query parameter required"));
            return;
        }
        if (!connector_param || connector_param->empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "merchant_connector_account_id query parameter required"));
            return;
        }
        const auto merchant_id = *merchant_param;
        const auto merchant_connector_id = *connector_param;

        const auto authenticated_merchant = authenticated_merchant_id(auth, req);
        if (!authenticated_merchant.empty() && authenticated_merchant != merchant_id) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "merchant id does not match authenticated account"));
            return;
        }

        // get_verified_apple_domains_with_mid_mca_id: read the MCA's persisted
        // applepay_verified_domains list. Cache is only a read-through mirror.
        auto repository = m_context->repository();
        auto connector_record = repository.find_merchant_connector_by_string(merchant_connector_id);
        if (!connector_record) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "merchant connector account not found"));
            return;
        }
        if (connector_record->get_merchant_id() != merchant_id) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "merchant connector account belongs to another merchant"));
            return;
        }
        if (const auto profile_error = validate_profile_id(auth.profile_id(), connector_record->get_profile_id())) {
            send_error(res, send, *profile_error);
            return;
        }
        const auto verified_domains = decode_domains(connector_record->get_applepay_verified_domains());

        m_context->cache().put(
            "verification.retrieve_apple_pay_verified_domains:" + merchant_id + ":" + merchant_connector_id,
            core::cache::to_cache_value(std::to_string(verified_domains.size()))
        );
        m_context->metrics().record_flow("Verification");
        m_context->metrics().counter("VERIFICATION").increment();
        m_context->metrics().record_flow("RetrieveApplePayVerifiedDomains");
        m_context->metrics().counter("APPLE_PAY_VERIFIED_DOMAINS_RETRIEVED").increment();

        app::dto::verifications::ApplepayVerifiedDomainsResponse response{};
        response.set_verified_domains(verified_domains);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Upstream v1 scope is /verify (app.rs:2870), not /verification.
        Router{router, "/verify"}
            .add_route(
                Route{"/apple_pay/:merchant_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_apple_pay_merchant_registration(req, res, std::move(send));
                    })
                    .summary("Hyperswitch apple_pay_merchant_registration")
            )
            .add_route(
                Route{"/applepay_verified_domains"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_apple_pay_verified_domains(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_apple_pay_verified_domains")
            );

        // Upstream v2 scope is /v2/verify (app.rs:2886).
        Router{router, "/v2/verify"}
            .add_route(
                Route{"/apple-pay/:merchant_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_apple_pay_merchant_registration(req, res, std::move(send));
                    })
                    .summary("Hyperswitch apple_pay_merchant_registration (v2)")
            )
            .add_route(
                Route{"/applepay-verified-domains"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_apple_pay_verified_domains(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_apple_pay_verified_domains (v2)")
            );
    }

    explicit VerificationRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
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

    /** @brief Registry rule first (API key), then JWT fallback.
     *
     *  Upstream accepts either API-key auth or JWT for these two routes
     *  (verification.rs:43-54,92-103,119-128,158-166). The registry row is the
     *  authoritative primary rule; a JWT rule with the same permission/connected
     *  flags is the explicit second branch.
     */
    [[nodiscard]] std::optional<core::error::AppError> authorize_verification(
        const app::auth::AuthContext& auth,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        if (const auto primary = m_context->authorizer().authorize_route(auth, rule)) {
            const auto jwt_rule = app::auth::RouteAuthRuleBuilder{}
                                      .scheme(app::auth::AuthScheme::JWT)
                                      .permission(rule.permission())
                                      .allow_connected()
                                      .build();
            return m_context->authorizer().authorize_route(auth, jwt_rule);
        }
        return std::nullopt;
    }

    /** @brief Local validate_profile_id_from_auth_layer substitute. */
    [[nodiscard]] std::optional<core::error::AppError> validate_profile_id(
        std::string_view authenticated_profile,
        std::string_view mca_profile
    ) const
    {
        if (!authenticated_profile.empty() && !mca_profile.empty() && authenticated_profile != mca_profile) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "profile id does not match merchant connector account"
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string authenticated_merchant_id(
        const app::auth::AuthContext& auth,
        interfaces::io::IRequest& req
    ) const
    {
        if (!auth.merchant_id().empty()) {
            return auth.merchant_id();
        }
        if (const auto connected = auth.claim("x-connected-merchant-id")) {
            if (!connected->empty()) {
                return std::string{*connected};
            }
        }
        return app::http::merchant_id_from_header{}(req);
    }

    [[nodiscard]] std::vector<std::string> parse_domain_names(const serde::Value& value) const
    {
        const auto* object = std::get_if<interfaces::Value::Object>(&value.get());
        if (object == nullptr) {
            return {};
        }
        const auto field = object->get("domain_names");
        if (!field) {
            return {};
        }
        std::vector<std::string> domains{};
        if (const auto* array = std::get_if<interfaces::Value::Array>(&field.value().get())) {
            for (const auto& element : *array) {
                append_string_value(domains, element);
            }
        }
        return domains;
    }

    [[nodiscard]] std::string parse_merchant_connector_id(const serde::Value& value) const
    {
        const auto* object = std::get_if<interfaces::Value::Object>(&value.get());
        if (object == nullptr) {
            return {};
        }
        const auto field = object->get("merchant_connector_account_id");
        if (!field) {
            return {};
        }
        std::vector<std::string> ids{};
        append_string_value(ids, field.value());
        return ids.empty() ? std::string{} : ids.front();
    }

    void append_string_value(std::vector<std::string>& target, const interfaces::Value& value) const
    {
        if (const auto* text = std::get_if<std::string>(&value.get())) {
            target.push_back(*text);
            return;
        }
        if (const auto* wrapper = std::get_if<interfaces::Value::Object>(&value.get())) {
            const auto inner = wrapper->get("value");
            if (inner) {
                if (const auto* wrapped = std::get_if<std::string>(&inner.value().get())) {
                    target.push_back(*wrapped);
                }
            }
        }
    }

    [[nodiscard]] std::string encode_domains(const std::vector<std::string>& domains) const
    {
        return serde::Ser::encode_json(domains);
    }

    [[nodiscard]] std::vector<std::string> decode_domains(std::string_view raw) const
    {
        std::vector<std::string> domains{};
        if (raw.empty()) {
            return domains;
        }
        auto parsed = serde::Ser::decode_generic("application/json", raw);
        if (!parsed) {
            return domains;
        }
        if (const auto* array = std::get_if<interfaces::Value::Array>(&parsed->get())) {
            for (const auto& element : *array) {
                append_string_value(domains, element);
            }
        }
        return domains;
    }

    [[nodiscard]] std::vector<std::string> merge_domains(
        std::vector<std::string> existing,
        const std::vector<std::string>& incoming
    ) const
    {
        for (const auto& domain : incoming) {
            if (std::find(existing.begin(), existing.end(), domain) == existing.end()) {
                existing.push_back(domain);
            }
        }
        return existing;
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::verification
