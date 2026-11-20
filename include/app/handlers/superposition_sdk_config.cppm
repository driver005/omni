module;
export module app_handlers:superposition_sdk_config;

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
import app_store;
import app_http;
import app_crypto;
import app_metrics;
import app_services;

export namespace app::handlers::superposition_sdk_config {

class SuperpositionSdkConfigRouteHandlers
{
public:
    void handle_get_sdk_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: ApiKeyAuth{allow_connected_scope_operation, allow_platform_self_operation}
        // via check_sdk_auth_and_get_auth (routes:52-95). The registry row for "get_sdk_config"
        // carries that scheme and both override flags (registry.cppm:174).
        const auto auth_ctx = authorize_request(req, res, send, "get_sdk_config");
        if (!auth_ctx) {
            return;
        }

        using Input = app::dto::superposition_sdk_config::SdkConfigRequest;
        using Response = app::dto::superposition_sdk_config::SuperPositionConfigResponse;
        using PaymentMethod = app::dto::superposition_sdk_config::SdkPaymentMethod;

        const auto sdk_auth = decode_sdk_authorization(req);

        // Upstream takes client_secret from the query string (web::Query<SdkConfigRequest>),
        // not the body. Body and decoded SDK auth are accepted as deterministic fallbacks.
        std::string client_secret{};
        const auto query = app::http::RequestQuery{}(req);
        if (const auto from_query = query.get("client_secret")) {
            client_secret = *from_query;
        } else {
            const std::string body = req.body_as_string();
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (parsed && parsed->get_client_secret()) {
                client_secret = parsed->get_client_secret()->get_value();
            }
        }
        if (client_secret.empty() && sdk_auth && sdk_auth->decoded) {
            client_secret = sdk_auth->client_secret;
        }
        if (client_secret.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST, "missing client_secret"
            ));
            return;
        }

        // Upstream get_payment_id_from_headers_or_payload(): the SDK Authorization header
        // carries payment_id; otherwise the id is split from client_secret at "_secret_".
        std::string payment_id{};
        if (sdk_auth && sdk_auth->decoded && !sdk_auth->payment_id.empty()) {
            payment_id = sdk_auth->payment_id;
        } else if (const auto from_secret = payment_id_from_client_secret(client_secret)) {
            payment_id = *from_secret;
        }
        if (payment_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST, "missing payment_id"
            ));
            return;
        }

        // Upstream find_payment_intent_by_payment_id_processor_merchant_id (core:360-368).
        // Scoped lookup by the string payment id; the former first-row fallback is gone.
        const auto stored_payment = m_context->repository().find_payment_by_string(payment_id);
        if (!stored_payment) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND, "payment not found"
            ));
            return;
        }
        const auto& payment = *stored_payment;
        const std::string merchant_id = payment.get_merchant_id();

        // Upstream get_payment_context(): attempt, addresses, customer and is_cit_transaction.
        // Address entities are not part of AppRepository yet (see shared-file follow-up);
        // attempt/customer/mandate lookups below are the deterministic local substitute.
        const auto attempt = find_payment_attempt(payment_id);
        const bool has_customer = !payment.get_customer_id().empty() &&
            m_context->repository().find_customer_by_string(payment.get_customer_id()).has_value();
        const bool is_cit_transaction = has_customer &&
            !m_context->repository().find_mandates_by_customer_id(payment.get_customer_id()).empty();

        std::string profile_id{};
        if (sdk_auth && sdk_auth->decoded) {
            profile_id = sdk_auth->profile_id;
        }
        if (profile_id.empty()) {
            profile_id = auth_ctx->profile_id();
        }
        if (profile_id.empty()) {
            profile_id = json_string_field(parse_json(payment.get_metadata()), "profile_id");
        }
        if (profile_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND, "profile id not found"
            ));
            return;
        }

        // Upstream find_business_profile_by_profile_id (core:378-384).
        const auto profile = m_context->repository().find_profile_by_string(profile_id);
        if (!profile) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND, "profile not found"
            ));
            return;
        }

        // Upstream list_enabled_merchant_connector_accounts_without_encrypted_by_merchant_id_profile_id
        // (core:163-170). Local filter over the repository MCA records.
        const auto enabled_mcas = enabled_merchant_connectors(merchant_id, profile_id);
        const auto payment_methods = build_payment_methods(enabled_mcas);

        // The payment attempt's connector is a deterministic fallback for the eligible
        // connector set when no enabled MCA carries payment_methods_enabled data.
        std::vector<std::string> connectors = eligible_connectors(enabled_mcas);
        if (connectors.empty() && attempt && !attempt->get_connector().empty()) {
            connectors.push_back(attempt->get_connector());
        }
        const auto merchant = m_context->repository().find_merchant_account_by_string(merchant_id);
        const std::string org_id = !auth_ctx->org_id().empty()
            ? auth_ctx->org_id()
            : (merchant ? merchant->get_organization_id() : std::string{});
        const std::string provider_merchant_id = app::http::HeaderReader{}.get(
            req, "x-provider-merchant-id"
        );

        // Upstream dimension filter with [DYNAMIC_FIELDS, CHECKOUT_SDK] dimensions
        // (core:424-433). connector[] is derived from the eligible MCA connectors.
        const auto filter = build_dimension_filter(
            profile_id,
            merchant_id,
            provider_merchant_id,
            org_id,
            connectors
        );

        // is_cit_transaction (computed from attempt/customer/mandate state) narrows the
        // superposition lookup the same way upstream eligibility narrows the config context.
        const std::string config_key = is_cit_transaction
            ? filter_key_with_cit(filter.key)
            : filter.key;

        Response response{};
        app::dto::superposition_sdk_config::ConfigValue raw_configs{};
        raw_configs.set_value(resolve_raw_config(config_key));
        response.set_raw_configs(std::move(raw_configs));
        // resolved_configs stays nullopt, matching upstream's `resolved_configs: None`.
        response.set_context_used(filter.value);
        response.set_payment_methods(std::vector<PaymentMethod>{payment_methods});
        response.set_account_config(build_account_config(profile_id, merchant_id, org_id));

        m_context->metrics().record_flow("GetSuperpositionSdkConfig");
        m_context->metrics().counter("get_superposition_sdk_config").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_get_profile_sdk_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream HeaderAuth(PublishableKeyAuth{...}) (routes:98-126). The registry row for
        // "get_profile_sdk_config" is the canonical rule (registry.cppm:165).
        const auto auth_ctx = authorize_request(req, res, send, "get_profile_sdk_config");
        if (!auth_ctx) {
            return;
        }

        // Route: /v1/sdk/configs/:platform/:profile_id/sdk_config.json
        // Upstream takes profile_id from the path and ignores any payload.
        const std::string profile_id = profile_id_from_path(req);
        if (profile_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST, "missing profile_id"
            ));
            return;
        }

        // Upstream find_business_profile_by_profile_id (core:152-161).
        const auto profile = m_context->repository().find_profile_by_string(profile_id);
        if (!profile) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND, "profile not found"
            ));
            return;
        }
        const std::string merchant_id = !profile->get_merchant_id().empty()
            ? profile->get_merchant_id()
            : auth_ctx->merchant_id();

        // Upstream list_enabled_merchant_connector_accounts_without_encrypted_by_merchant_id_profile_id
        // (core:163-170), then build_merchant_enabled_pms_context (core:172-289).
        const auto enabled_mcas = enabled_merchant_connectors(merchant_id, profile_id);
        const auto payment_methods = build_payment_methods(enabled_mcas);

        const auto merchant = m_context->repository().find_merchant_account_by_string(merchant_id);
        const std::string org_id = !auth_ctx->org_id().empty()
            ? auth_ctx->org_id()
            : (merchant ? merchant->get_organization_id() : std::string{});
        const std::string provider_merchant_id = app::http::HeaderReader{}.get(
            req, "x-provider-merchant-id"
        );

        const auto filter = build_dimension_filter(
            profile_id,
            merchant_id,
            provider_merchant_id,
            org_id,
            eligible_connectors(enabled_mcas)
        );

        using Response = app::dto::superposition_sdk_config::SuperPositionConfigResponse;
        using PaymentMethod = app::dto::superposition_sdk_config::SdkPaymentMethod;
        Response response{};
        app::dto::superposition_sdk_config::ConfigValue raw_configs{};
        raw_configs.set_value(resolve_raw_config(filter.key));
        response.set_raw_configs(std::move(raw_configs));
        response.set_context_used(filter.value);
        response.set_payment_methods(std::vector<PaymentMethod>{payment_methods});
        response.set_account_config(build_account_config(profile_id, merchant_id, org_id));

        m_context->metrics().record_flow("GetSuperpositionSdkConfig");
        m_context->metrics().counter("get_profile_superposition_sdk_config").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/superposition_sdk_config"}
            .add_route(
                Route{"/v1/sdk/configs/:platform/sdk_config.json"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_sdk_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_sdk_config")
            )
            .add_route(
                Route{"/v1/sdk/configs/:platform/:profile_id/sdk_config.json"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_profile_sdk_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_profile_sdk_config")
            );
    }

    explicit SuperpositionSdkConfigRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    struct SdkAuth
    {
        std::string profile_id{};
        std::string payment_id{};
        std::string publishable_key{};
        std::string client_secret{};
        bool decoded{false};
    };

    struct DimensionFilter
    {
        serde::Value value{};
        std::string json{};
        std::string key{};
    };

    struct CriteriaRules
    {
        std::optional<app::dto::superposition_sdk_config::PaymentMethodCriteria> criteria{};
        std::vector<app::dto::superposition_sdk_config::SdkCriteriaRule> rules{};
    };

    using PmKey = std::pair<std::string, std::string>;

    [[nodiscard]] std::optional<app::auth::AuthContext> authorize_request(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        auto ctx = app::auth::AuthContextBuilder{}(req);

        // SdkAuthorization::decode (domain_models/sdk_auth.rs:134-236) is a base64-wrapped
        // comma-separated key=value map. AuthContextBuilder only reads the plain form, so
        // decode the wrapped form here and feed the same claims into the authorizer.
        const auto sdk_auth = decode_sdk_authorization(req);
        if (sdk_auth && sdk_auth->decoded) {
            if (!sdk_auth->publishable_key.empty()) {
                ctx.add_claim(std::string{app::auth::kClaimPublishableKey}, sdk_auth->publishable_key);
            }
            if (!sdk_auth->client_secret.empty()) {
                ctx.add_claim(std::string{app::auth::kClaimClientSecret}, sdk_auth->client_secret);
            }
            if (!sdk_auth->profile_id.empty()) {
                ctx.add_claim("x-profile-id", sdk_auth->profile_id);
                ctx.set_profile_id(sdk_auth->profile_id);
            }
        }

        // The registry exposes these SDK routes through bearer-based schemes even though
        // upstream authenticates with SDK/publishable headers. Feed the presented header
        // credential as the flat scheme's credential; the header claims are still required
        // by authorize_route for SDK/PUBLISHABLE_KEY rules.
        if (ctx.token().is_empty()) {
            const auto sdk_key = ctx.claim(app::auth::kClaimSdkKey);
            const auto client_secret = ctx.claim(app::auth::kClaimClientSecret);
            const auto publishable_key = ctx.claim(app::auth::kClaimPublishableKey);
            if (sdk_key && !sdk_key->empty()) {
                ctx.set_token(app::auth::BearerToken{std::string{*sdk_key}});
            } else if (client_secret && !client_secret->empty()) {
                ctx.set_token(app::auth::BearerToken{std::string{*client_secret}});
            } else if (publishable_key && !publishable_key->empty()) {
                ctx.set_token(app::auth::BearerToken{std::string{*publishable_key}});
            }
        }

        const auto auth = m_context->authorizer().authorize_route(ctx, rule);
        if (auth) {
            send_error(res, send, *auth);
            return std::nullopt;
        }
        // Header-only SDK auth must still present the publishable key, as upstream's
        // PublishableKeyAuth requires (routes:64-77).
        if (operation == "get_profile_sdk_config") {
            const auto publishable_key = ctx.claim(app::auth::kClaimPublishableKey);
            if (!publishable_key.has_value() || publishable_key->empty()) {
                send_error(res, send, core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::UNAUTHORIZED, "missing publishable key header"
                ));
                return std::nullopt;
            }
        }
        return ctx;
    }

    [[nodiscard]] std::optional<SdkAuth> decode_sdk_authorization(
        interfaces::io::IRequest& req
    ) const
    {
        const std::string header = app::http::HeaderReader{}.get(req, "authorization");
        if (header.empty()) {
            return std::nullopt;
        }
        constexpr std::string_view bearer_prefix{"Bearer "};
        if (header.size() > bearer_prefix.size() &&
            std::equal(bearer_prefix.begin(), bearer_prefix.end(), header.begin(),
                [](char lhs, char rhs) {
                    return std::tolower(static_cast<unsigned char>(lhs)) ==
                           std::tolower(static_cast<unsigned char>(rhs));
                })) {
            return std::nullopt;
        }

        // BASE64_ENGINE is standard base64 with padding (common_utils/consts.rs:124);
        // Base64Url is the local codec, so translate the alphabet and drop padding.
        std::string normalized{};
        normalized.reserve(header.size());
        for (const char ch : header) {
            if (ch == '+') {
                normalized.push_back('-');
            } else if (ch == '/') {
                normalized.push_back('_');
            } else if (ch != '=') {
                normalized.push_back(ch);
            }
        }
        std::string decoded = app::crypto::Base64Url{}.decode(normalized);
        if (decoded.find('=') == std::string::npos) {
            decoded = header;
        }

        SdkAuth result{};
        std::size_t start{0};
        while (start <= decoded.size()) {
            const std::size_t comma = decoded.find(',', start);
            const std::string_view pair = std::string_view{decoded}.substr(
                start, comma == std::string::npos ? std::string::npos : comma - start
            );
            const std::size_t equals = pair.find('=');
            if (equals != std::string_view::npos) {
                const std::string key = trim_copy(pair.substr(0, equals));
                const std::string value = trim_copy(pair.substr(equals + 1));
                if (!key.empty() && !value.empty()) {
                    result.decoded = true;
                    if (key == "profile_id") {
                        result.profile_id = value;
                    } else if (key == "payment_id") {
                        result.payment_id = value;
                    } else if (key == "publishable_key" || key == "publishable-key") {
                        result.publishable_key = value;
                    } else if (key == "client_secret" || key == "client-secret") {
                        result.client_secret = value;
                    }
                }
            }
            if (comma == std::string::npos) {
                break;
            }
            start = comma + 1;
        }
        return result;
    }

    [[nodiscard]] std::string trim_copy(std::string_view value) const
    {
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
            value.remove_prefix(1);
        }
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
            value.remove_suffix(1);
        }
        return std::string{value};
    }

    // Mirrors payments::helpers::get_payment_id_from_client_secret (helpers.rs:5172-5177):
    // rsplit_once("_secret_"). Kept local because the helper is not exported by a core module.
    [[nodiscard]] std::optional<std::string> payment_id_from_client_secret(
        std::string_view client_secret
    ) const
    {
        constexpr std::string_view separator{"_secret_"};
        const auto split = client_secret.rfind(separator);
        if (split == std::string_view::npos || split == 0 ||
            split + separator.size() >= client_secret.size()) {
            return std::nullopt;
        }
        return std::string{client_secret.substr(0, split)};
    }

    [[nodiscard]] std::optional<app::store::payment_attempts::PaymentAttemptRecord>
    find_payment_attempt(std::string_view payment_id) const
    {
        for (const auto& attempt : m_context->repository().list_payment_attempts()) {
            if (attempt.get_payment_id() == payment_id) {
                return attempt;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::vector<app::store::merchant_connectors::MerchantConnectorRecord>
    enabled_merchant_connectors(std::string_view merchant_id, std::string_view profile_id) const
    {
        std::vector<app::store::merchant_connectors::MerchantConnectorRecord> result{};
        for (const auto& mca : m_context->repository().list_merchant_connectors()) {
            if (mca.get_disabled() || !mca.get_status().empty() && mca.get_status() != "active") {
                continue;
            }
            if (!merchant_id.empty() && mca.get_merchant_id() != merchant_id) {
                continue;
            }
            if (!profile_id.empty() && !mca.get_profile_id().empty() &&
                mca.get_profile_id() != profile_id) {
                continue;
            }
            result.push_back(mca);
        }
        return result;
    }

    [[nodiscard]] std::string profile_id_from_path(interfaces::io::IRequest& req) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        for (std::size_t i = 0; i < segments.count(); ++i) {
            if (segments.at(i).value_or("") == "configs" && i + 2 < segments.count()) {
                return std::string{segments.at(i + 2).value_or("")};
            }
        }
        for (std::size_t i = 1; i < segments.count(); ++i) {
            if (segments.at(i).value_or("") == "sdk_config.json") {
                return std::string{segments.at(i - 1).value_or("")};
            }
        }
        return segments.count() > 0 ? std::string{segments.at(segments.count() - 1).value_or("")}
                                    : std::string{};
    }

    [[nodiscard]] std::optional<serde::Value> parse_json(std::string_view text) const
    {
        if (text.empty()) {
            return std::nullopt;
        }
        auto decoded = serde::Ser::decode_generic("application/json", text);
        if (!decoded) {
            return std::nullopt;
        }
        return std::move(*decoded);
    }

    [[nodiscard]] std::string json_string(const serde::Value& value) const
    {
        if (const auto* text = std::get_if<std::string>(&value.get())) {
            return *text;
        }
        return {};
    }

    [[nodiscard]] std::string json_string_field(
        const std::optional<serde::Value>& object,
        std::string_view key
    ) const
    {
        if (!object) {
            return {};
        }
        return json_string_field(*object, key);
    }

    [[nodiscard]] std::string json_string_field(
        const serde::Value& value,
        std::string_view key
    ) const
    {
        const auto* object = std::get_if<serde::Value::Object>(&value.get());
        if (object == nullptr) {
            return {};
        }
        const auto field = object->get(std::string{key});
        if (!field) {
            return {};
        }
        return json_string(field.value());
    }

    [[nodiscard]] std::vector<serde::Value> json_array(
        const serde::Value& value,
        std::string_view key
    ) const
    {
        std::vector<serde::Value> result{};
        const auto* object = std::get_if<serde::Value::Object>(&value.get());
        if (object == nullptr) {
            return result;
        }
        const auto field = object->get(std::string{key});
        if (!field) {
            return result;
        }
        const auto* array = std::get_if<serde::Value::Array>(&field.value().get());
        if (array == nullptr) {
            return result;
        }
        for (const auto& element : *array) {
            result.push_back(element);
        }
        return result;
    }

    void add_rule(
        std::map<PmKey, CriteriaRules>& consolidated,
        std::string_view payment_method,
        std::string_view payment_method_type,
        std::optional<app::dto::superposition_sdk_config::PaymentMethodCriteria> criteria,
        std::string_view criteria_value,
        std::string_view connector
    ) const
    {
        if (payment_method.empty() || payment_method_type.empty() || criteria_value.empty() ||
            connector.empty()) {
            return;
        }
        auto& entry = consolidated[PmKey{std::string{payment_method}, std::string{payment_method_type}}];
        if (criteria.has_value()) {
            entry.criteria = criteria;
        }
        for (auto& rule : entry.rules) {
            if (rule.get_criteria_value() == criteria_value) {
                auto connectors = rule.get_eligible_connectors();
                if (std::find(connectors.begin(), connectors.end(), connector) == connectors.end()) {
                    connectors.emplace_back(connector);
                    rule.set_eligible_connectors(std::move(connectors));
                }
                return;
            }
        }
        using Rule = app::dto::superposition_sdk_config::SdkCriteriaRule;
        Rule rule{};
        rule.set_criteria_value(std::string{criteria_value});
        rule.set_eligible_connectors(std::vector<std::string>{std::string{connector}});
        entry.rules.push_back(std::move(rule));
    }

    // Upstream build_merchant_enabled_pms_context (core:172-289, 386-397) and
    // translate_to_sdk_payment_methods (core:564-688). MCA payment_methods_enabled JSON is
    // consolidated into payment-experience / card-network / bank criteria maps, then sorted.
    [[nodiscard]] std::vector<app::dto::superposition_sdk_config::SdkPaymentMethod>
    build_payment_methods(
        const std::vector<app::store::merchant_connectors::MerchantConnectorRecord>& mcas
    ) const
    {
        using namespace app::dto::superposition_sdk_config;
        std::map<PmKey, CriteriaRules> consolidated{};
        for (const auto& mca : mcas) {
            const auto parsed = parse_json(mca.get_payment_methods_enabled());
            if (!parsed) {
                continue;
            }
            const auto* methods = std::get_if<serde::Value::Array>(&parsed->get());
            if (methods == nullptr) {
                continue;
            }
            const std::string connector = mca.get_connector_name();
            for (const auto& method_value : *methods) {
                const std::string payment_method = json_string_field(method_value, "payment_method");
                if (payment_method.empty()) {
                    continue;
                }
                for (const auto& type_value : json_array(method_value, "payment_method_types")) {
                    const std::string pmt = json_string_field(type_value, "payment_method_type");
                    if (pmt.empty()) {
                        continue;
                    }
                    const std::string experience = json_string_field(type_value, "payment_experience");
                    if (!experience.empty()) {
                        add_rule(consolidated, payment_method, pmt,
                                 PaymentMethodCriteria::PaymentExperience, experience, connector);
                    }
                    for (const auto& network : json_array(type_value, "card_networks")) {
                        add_rule(consolidated, payment_method, pmt,
                                 PaymentMethodCriteria::CardNetwork, json_string(network), connector);
                    }
                    if (payment_method == "bank_redirect") {
                        // get_banks() has no local bank catalogue; the connector name is the
                        // deterministic bank-name substitute (documented in the parity report).
                        add_rule(consolidated, payment_method, pmt,
                                 PaymentMethodCriteria::BankName, connector, connector);
                    } else if (payment_method == "bank_debit" || payment_method == "bank_transfer") {
                        add_rule(consolidated, payment_method, pmt,
                                 std::nullopt, "default", connector);
                    }
                }
            }
        }

        std::vector<std::pair<PmKey, CriteriaRules>> entries{consolidated.begin(), consolidated.end()};
        std::sort(entries.begin(), entries.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.first < rhs.first;
        });
        std::vector<SdkPaymentMethod> result{};
        for (auto& [key, rules] : entries) {
            if (rules.rules.empty()) {
                continue;
            }
            std::sort(rules.rules.begin(), rules.rules.end(), [](const auto& lhs, const auto& rhs) {
                return lhs.get_criteria_value() < rhs.get_criteria_value();
            });
            SdkPaymentMethodType type{};
            PaymentMethodTypeValue type_value{};
            type_value.set_value(key.second);
            type.set_payment_method_type(std::move(type_value));
            type.set_payment_method_criteria(rules.criteria);
            type.set_criteria_rules(std::move(rules.rules));
            SdkPaymentMethod method{};
            PaymentMethodValue method_value{};
            method_value.set_value(key.first);
            method.set_payment_method(std::move(method_value));
            method.set_payment_method_types(std::vector<SdkPaymentMethodType>{std::move(type)});
            result.push_back(std::move(method));
        }
        return result;
    }

    [[nodiscard]] std::vector<std::string> eligible_connectors(
        const std::vector<app::store::merchant_connectors::MerchantConnectorRecord>& mcas
    ) const
    {
        std::vector<std::string> result{};
        for (const auto& mca : mcas) {
            const std::string& connector = mca.get_connector_name();
            if (connector.empty() || std::find(result.begin(), result.end(), connector) != result.end()) {
                continue;
            }
            result.push_back(connector);
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    [[nodiscard]] std::string json_quote(std::string_view value) const
    {
        std::string out{"\""};
        for (const char ch : value) {
            switch (ch) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(ch); break;
            }
        }
        out.push_back('"');
        return out;
    }

    [[nodiscard]] DimensionFilter build_dimension_filter(
        std::string_view profile_id,
        std::string_view processor_merchant_id,
        std::string_view provider_merchant_id,
        std::string_view organization_id,
        const std::vector<std::string>& connectors
    ) const
    {
        std::string connectors_json{"["};
        for (std::size_t i = 0; i < connectors.size(); ++i) {
            if (i > 0) {
                connectors_json.push_back(',');
            }
            connectors_json += json_quote(connectors[i]);
        }
        connectors_json.push_back(']');

        DimensionFilter filter{};
        filter.json = std::format(
            R"({{"profile_id":{},"processor_merchant_id":{},"provider_merchant_id":{},"organization_id":{},"connector":{}}})",
            json_quote(profile_id),
            json_quote(processor_merchant_id),
            json_quote(provider_merchant_id),
            json_quote(organization_id),
            connectors_json
        );
        auto decoded = parse_json(filter.json);
        filter.value = decoded ? std::move(*decoded) : serde::Value{};
        filter.key = app::crypto::KeyedHasher{"superposition_sdk_config"}.hash(filter.json);
        return filter;
    }

    [[nodiscard]] std::string filter_key_with_cit(std::string_view dimension_key) const
    {
        return std::string{dimension_key} + ":cit";
    }

    // Upstream superposition_service.get_cached_config([DYNAMIC_FIELDS, CHECKOUT_SDK], filter)
    // (core:324-336, 435-447). SuperpositionClient is the repository-backed local service;
    // the dimension hash keys the checkout_sdk config lookup without storing the raw secret.
    [[nodiscard]] std::string resolve_raw_config(std::string_view dimension_key) const
    {
        const auto record = m_context->superposition().resolve(
            "dynamic_fields", "checkout_sdk", dimension_key
        );
        if (!record || record->get_value().empty()) {
            return R"({"configs":{}})";
        }
        const std::string& value = record->get_value();
        if (value.front() == '{' && value.back() == '}') {
            return value;
        }
        return std::format(R"({{"configs":{}}})", json_quote(value));
    }

    [[nodiscard]] bool config_flag(
        std::string_view config_namespace,
        std::string_view key,
        bool default_value
    ) const
    {
        const auto record = m_context->superposition().get_config(config_namespace, key);
        if (!record) {
            return default_value;
        }
        return record->get_value() == "true" || record->get_value() == "1";
    }

    [[nodiscard]] app::dto::superposition_sdk_config::VaultingAction resolve_vaulting_action(
        bool should_call_modular,
        bool should_perform_sdk_vaulting
    ) const
    {
        using VaultingAction = app::dto::superposition_sdk_config::VaultingAction;
        return should_call_modular && should_perform_sdk_vaulting
            ? VaultingAction::Tokenize
            : VaultingAction::Skip;
    }

    // Upstream build_account_config / build_profile_account_config (core:465-523). Profile
    // wallet flags default to false (unwrap_or(false)); both vaulting gates default to true.
    [[nodiscard]] app::dto::superposition_sdk_config::AccountConfig build_account_config(
        std::string_view profile_id,
        std::string_view merchant_id,
        std::string_view organization_id
    ) const
    {
        using namespace app::dto::superposition_sdk_config;
        const auto profile_flag = [this, profile_id](std::string_view flag) {
            const std::string key{std::string{profile_id} + "." + std::string{flag}};
            return config_flag("sdk_account_config", key, false);
        };
        ProfileAccountConfig profile{};
        profile.set_collect_shipping_details_from_wallet_connector(
            profile_flag("collect_shipping_details_from_wallet_connector"));
        profile.set_collect_billing_details_from_wallet_connector(
            profile_flag("collect_billing_details_from_wallet_connector"));
        profile.set_always_collect_billing_details_from_wallet_connector(
            profile_flag("always_collect_billing_details_from_wallet_connector"));
        profile.set_always_collect_shipping_details_from_wallet_connector(
            profile_flag("always_collect_shipping_details_from_wallet_connector"));

        const bool modular = config_flag(
            "feature_config",
            std::string{merchant_id} + ".is_payment_method_modular_allowed",
            true
        );
        const std::string vaulting_key = organization_id.empty()
            ? std::string{merchant_id} + ".should_perform_sdk_vaulting"
            : std::string{organization_id} + ".should_perform_sdk_vaulting";
        const bool vaulting = config_flag("feature_config", vaulting_key, true);
        profile.set_vaulting_action(resolve_vaulting_action(modular, vaulting));

        AccountConfig account_config{};
        account_config.set_profile(std::move(profile));
        return account_config;
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

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::superposition_sdk_config
