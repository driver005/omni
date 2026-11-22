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

/**
 * Parity implementation of upstream
 * `crates/router/src/routes/superposition_sdk_config.rs` and
 * `crates/router/src/core/superposition_sdk_config.rs`.
 *
 * Every upstream operation is ported as a local helper of this translation unit. External
 * infrastructure (the superposition HTTP service, the bank catalogue, the address store)
 * is replaced by in-process facilities that implement the same control flow against
 * AppContext/AppRepository/SuperpositionClient, as required by the local parity contract.
 */
class SuperpositionSdkConfigRouteHandlers
{
public:
    // ---------------------------------------------------------------- routes ----------------------------------------------------------------

    // Upstream routes::get_sdk_config (routes:52-95).
    void handle_get_sdk_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream step 1 (routes:63-66): get_payment_id_from_headers_or_payload, before auth.
        const std::string auth_header = app::http::HeaderReader{}.get(req, "authorization");
        const auto sdk_auth = decode_sdk_authorization(auth_header);
        const auto query = app::http::RequestQuery{}(req);
        const auto resolved = get_payment_id_from_headers_or_payload(auth_header, sdk_auth, query);
        if (resolved.error) {
            send_api_error(res, send, *resolved.error);
            return;
        }
        const std::string payment_id = resolved.value->payment_id;
        const std::string client_secret = resolved.value->client_secret;

        // Upstream step 2 (routes:68-77): check_sdk_auth_and_get_auth with
        // ApiKeyAuth{allow_connected_scope_operation, allow_platform_self_operation}. The local
        // registry row for "get_sdk_config" is the canonical rule (registry.cppm:174).
        const auto auth_ctx = authorize_request(
            req, res, send, "get_sdk_config", client_secret, sdk_auth, !auth_header.empty()
        );
        if (!auth_ctx) {
            return;
        }

        using Response = app::dto::superposition_sdk_config::SuperPositionConfigResponse;

        // Upstream core::get_superposition_sdk_config (core:353-462).
        // find_payment_intent_by_payment_id_processor_merchant_id (core:360-368): the local
        // store has no processor-merchant composite key, so the id-scoped lookup is used.
        const auto stored_payment = m_context->repository().find_payment_by_string(payment_id);
        if (!stored_payment) {
            send_api_error(res, send, payment_not_found());
            return;
        }
        const auto& payment = *stored_payment;
        const std::string merchant_id = payment.get_merchant_id();

        // Upstream get_payment_context (core:35-137) -> SdkPaymentContext.
        const auto payment_context = get_payment_context(payment_id, payment);

        // Upstream payment_intent.profile_id.ok_or(GenericNotFoundError { "Profile id not found" })
        // (core:372-376). The local PaymentIntentRecord has no profile_id column, so the id is
        // taken from the SDK claim, the authenticated context, then the intent metadata.
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
            send_api_error(res, send, generic_not_found("Profile id not found"));
            return;
        }

        // Upstream find_business_profile_by_profile_id (core:378-384).
        const auto profile = m_context->repository().find_profile_by_string(profile_id);
        if (!profile) {
            send_api_error(res, send, profile_not_found(profile_id));
            return;
        }

        // Upstream list_enabled_merchant_connector_accounts_without_encrypted...
        // (core:163-170) and build_merchant_enabled_pms_context (core:386-397).
        const auto enabled_mcas = list_enabled_merchant_connector_accounts(merchant_id, profile_id);
        const auto pms_context = build_merchant_enabled_pms_context(enabled_mcas);
        const auto payment_methods = translate_to_sdk_payment_methods(pms_context);

        // Upstream dimension filter (core:399-433): connector[] comes directly from
        // MerchantEnabledPmsContext::get_eligible_connectors().
        const auto filter = build_dimension_filter(
            profile_id, merchant_id, provider_merchant_id(req), organization_id(merchant_id, *auth_ctx),
            pms_context.get_eligible_connectors()
        );

        // is_cit_transaction (core:122-128) feeds the local superposition context key: the
        // in-process SuperpositionClient has no eligibility dimension, so the CIT state narrows
        // the scoped lookup here.
        const std::string config_key = filter_key_with_cit(filter.key, payment_context.is_cit_transaction);

        Response response{};
        response.set_raw_configs(resolve_raw_config(config_key));
        response.set_context_used(filter.value);
        response.set_payment_methods(std::vector<app::dto::superposition_sdk_config::SdkPaymentMethod>{payment_methods});
        response.set_account_config(build_account_config(*profile, profile_id, merchant_id, *auth_ctx));

        m_context->metrics().record_flow("GetSuperpositionSdkConfig");
        m_context->metrics().counter("get_superposition_sdk_config").increment();

        send_json(res, send, serde::Ser::encode_json(response));
    }

    // Upstream routes::get_profile_sdk_config (routes:98-126).
    void handle_get_profile_sdk_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream HeaderAuth(PublishableKeyAuth{...}) (routes:119-122). Registry row:
        // registry.cppm:165.
        const auto auth_ctx = authorize_request(req, res, send, "get_profile_sdk_config", {}, std::nullopt, false);
        if (!auth_ctx) {
            return;
        }

        // Upstream path.into_inner() (routes:105); the payload is ignored.
        const std::string profile_id = profile_id_from_path(req);
        if (profile_id.empty()) {
            send_api_error(res, send, missing_required_field("profile_id"));
            return;
        }

        // Upstream find_business_profile_by_profile_id (core:152-161).
        const auto profile = m_context->repository().find_profile_by_string(profile_id);
        if (!profile) {
            send_api_error(res, send, profile_not_found(profile_id));
            return;
        }
        const std::string merchant_id = !profile->get_merchant_id().empty()
            ? profile->get_merchant_id()
            : auth_ctx->merchant_id();

        // Upstream list_enabled_merchant_connector_accounts_without_encrypted... (core:163-170)
        // and build_merchant_enabled_pms_context (core:172-289).
        const auto enabled_mcas = list_enabled_merchant_connector_accounts(merchant_id, profile_id);
        const auto pms_context = build_merchant_enabled_pms_context(enabled_mcas);
        const auto payment_methods = translate_to_sdk_payment_methods(pms_context);

        // Upstream dimension filter (core:291-322): connector[] is the raw mca connector list.
        const auto filter = build_dimension_filter(
            profile_id, merchant_id, provider_merchant_id(req), organization_id(merchant_id, *auth_ctx),
            enabled_connectors(enabled_mcas)
        );

        using Response = app::dto::superposition_sdk_config::SuperPositionConfigResponse;
        Response response{};
        response.set_raw_configs(resolve_raw_config(filter.key));
        response.set_context_used(filter.value);
        response.set_payment_methods(std::vector<app::dto::superposition_sdk_config::SdkPaymentMethod>{payment_methods});
        response.set_account_config(build_account_config(*profile, profile_id, merchant_id, *auth_ctx));

        m_context->metrics().record_flow("GetSuperpositionSdkConfig");
        m_context->metrics().counter("get_profile_superposition_sdk_config").increment();

        send_json(res, send, serde::Ser::encode_json(response));
    }

    // Upstream app.rs SdkConfig::server (app.rs:3584-3595): exactly the two GET routes below.
    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "superposition_sdk_config"}
            .add_router(Router{router, "v1"}.add_router(Router{router, "sdk"}.add_router(Router{router, "configs"}.add_router(Router{router, ":platform"}.add_route(
                Route{"sdk_config.json"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_sdk_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_sdk_config")
            )))))
            .add_router(Router{router, "v1"}.add_router(Router{router, "sdk"}.add_router(Router{router, "configs"}.add_router(Router{router, ":platform"}.add_router(Router{router, ":profile_id"}.add_route(
                Route{"sdk_config.json"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_profile_sdk_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_profile_sdk_config")
            ))))));
    }

    explicit SuperpositionSdkConfigRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // ---------------------------------------------------------------- upstream error model ----------------------------------------------------------------

    // Upstream ApiErrorResponse variants used by these two routes. The local core::error model
    // has no named variants, so the variant name is carried on the wire as `error` and mapped to
    // the upstream HTTP status in send_api_error.
    enum class ApiErrorVariant
    {
        Unauthorized,
        MissingRequiredField,
        InvalidDataValue,
        PaymentNotFound,
        ProfileNotFound,
        CustomerNotFound,
        MerchantAccountNotFound,
        GenericNotFoundError,
        InternalServerError,
    };

    struct ApiError
    {
        ApiErrorVariant variant{};
        std::string field{};
        std::string detail{};
    };

    [[nodiscard]] static ApiError unauthorized()
    {
        return ApiError{ApiErrorVariant::Unauthorized, {}, {}};
    }
    [[nodiscard]] static ApiError missing_required_field(std::string_view field)
    {
        return ApiError{ApiErrorVariant::MissingRequiredField, std::string{field}, {}};
    }
    [[nodiscard]] static ApiError invalid_data_value(std::string_view field)
    {
        return ApiError{ApiErrorVariant::InvalidDataValue, std::string{field}, {}};
    }
    [[nodiscard]] static ApiError payment_not_found()
    {
        return ApiError{ApiErrorVariant::PaymentNotFound, {}, {}};
    }
    [[nodiscard]] static ApiError profile_not_found(std::string_view id)
    {
        return ApiError{ApiErrorVariant::ProfileNotFound, {}, std::string{id}};
    }
    [[nodiscard]] static ApiError customer_not_found()
    {
        return ApiError{ApiErrorVariant::CustomerNotFound, {}, {}};
    }
    [[nodiscard]] static ApiError merchant_account_not_found()
    {
        return ApiError{ApiErrorVariant::MerchantAccountNotFound, {}, {}};
    }
    [[nodiscard]] static ApiError generic_not_found(std::string_view message)
    {
        return ApiError{ApiErrorVariant::GenericNotFoundError, {}, std::string{message}};
    }
    [[nodiscard]] static ApiError internal_server_error()
    {
        return ApiError{ApiErrorVariant::InternalServerError, {}, {}};
    }

    [[nodiscard]] std::string_view api_error_name(ApiErrorVariant variant) const noexcept
    {
        switch (variant) {
        case ApiErrorVariant::Unauthorized: return "Unauthorized";
        case ApiErrorVariant::MissingRequiredField: return "MissingRequiredField";
        case ApiErrorVariant::InvalidDataValue: return "InvalidDataValue";
        case ApiErrorVariant::PaymentNotFound: return "PaymentNotFound";
        case ApiErrorVariant::ProfileNotFound: return "ProfileNotFound";
        case ApiErrorVariant::CustomerNotFound: return "CustomerNotFound";
        case ApiErrorVariant::MerchantAccountNotFound: return "MerchantAccountNotFound";
        case ApiErrorVariant::GenericNotFoundError: return "GenericNotFoundError";
        case ApiErrorVariant::InternalServerError: return "InternalServerError";
        }
        return "InternalServerError";
    }

    // Upstream api::log_and_return_error_response (routes:65,76).
    void send_api_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        const ApiError& error
    ) const
    {
        using interfaces::io::types::Status;
        using core::error::AppErrorBuilder;
        using core::error::AppErrorKind;

        Status status{Status::BAD_REQUEST};
        AppErrorKind kind{AppErrorKind::INVALID_REQUEST};
        switch (error.variant) {
        case ApiErrorVariant::Unauthorized:
            status = Status::UNAUTHORIZED;
            kind = AppErrorKind::UNAUTHORIZED;
            break;
        case ApiErrorVariant::MissingRequiredField:
        case ApiErrorVariant::InvalidDataValue:
            status = Status::BAD_REQUEST;
            kind = AppErrorKind::VALIDATION_FAILED;
            break;
        case ApiErrorVariant::PaymentNotFound:
        case ApiErrorVariant::ProfileNotFound:
        case ApiErrorVariant::CustomerNotFound:
        case ApiErrorVariant::MerchantAccountNotFound:
        case ApiErrorVariant::GenericNotFoundError:
            status = Status::NOT_FOUND;
            kind = AppErrorKind::NOT_FOUND;
            break;
        case ApiErrorVariant::InternalServerError:
            status = Status::INTERNAL_SERVER_ERROR;
            kind = AppErrorKind::INTERNAL_ERROR;
            break;
        }

        std::string message{};
        switch (error.variant) {
        case ApiErrorVariant::MissingRequiredField:
            message = std::format("Missing required field: {}", error.field);
            break;
        case ApiErrorVariant::InvalidDataValue:
            message = std::format("Invalid data value: {}", error.field);
            break;
        case ApiErrorVariant::ProfileNotFound:
            message = std::format("Profile not found: {}", error.detail);
            break;
        case ApiErrorVariant::GenericNotFoundError:
            message = error.detail;
            break;
        default:
            message = std::string{api_error_name(error.variant)};
            break;
        }

        auto wire = AppErrorBuilder{}(kind, std::move(message));
        wire.set_kind_name(std::string{api_error_name(error.variant)});
        send_error(res, send, std::move(wire), status);
    }

    // ---------------------------------------------------------------- auth ----------------------------------------------------------------

    struct SdkAuth
    {
        std::string profile_id{};
        std::string payment_id{};
        std::string publishable_key{};
        std::string client_secret{};
        bool decoded{false};
    };

    // Upstream check_sdk_auth_and_get_auth (authentication.rs:6037-6066) plus the SDK-header
    // claim decoding of SdkAuthorization::decode (domain_models/sdk_auth.rs:134-236).
    [[nodiscard]] std::optional<app::auth::AuthContext> authorize_request(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view operation,
        std::string_view client_secret,
        const std::optional<SdkAuth>& sdk_auth,
        bool has_authorization_header
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        auto ctx = app::auth::AuthContextBuilder{}(req);

        // SdkAuthorization::decode yields a base64-wrapped comma-separated key=value map.
        // AuthContextBuilder only reads the plain form, so decode the wrapped form here and feed
        // the same claims into the authorizer.
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
        // Upstream check_client_secret_and_get_auth reads client_secret from the payload; the
        // local builder does not read the query string, so inject the resolved query value.
        if (!client_secret.empty()) {
            ctx.add_claim(std::string{app::auth::kClaimClientSecret}, std::string{client_secret});
        }

        // The registry exposes these SDK routes through bearer-based schemes even though
        // upstream authenticates with SDK/publishable headers. Feed the presented header
        // credential as the flat scheme's credential; the header claims are still required
        // by authorize_route for SDK/PUBLISHABLE_KEY rules.
        if (ctx.token().is_empty()) {
            const auto sdk_key = ctx.claim(app::auth::kClaimSdkKey);
            const auto secret_claim = ctx.claim(app::auth::kClaimClientSecret);
            const auto publishable_key = ctx.claim(app::auth::kClaimPublishableKey);
            if (sdk_key && !sdk_key->empty()) {
                ctx.set_token(app::auth::BearerToken{std::string{*sdk_key}});
            } else if (secret_claim && !secret_claim->empty()) {
                ctx.set_token(app::auth::BearerToken{std::string{*secret_claim}});
            } else if (publishable_key && !publishable_key->empty()) {
                ctx.set_token(app::auth::BearerToken{std::string{*publishable_key}});
            }
        }

        const auto auth = m_context->authorizer().authorize_route(ctx, rule);
        if (auth) {
            send_error(res, send, *auth);
            return std::nullopt;
        }

        // Upstream HeaderAuth(PublishableKeyAuth) for the profile route (routes:119-122);
        // SdkAuthorizationAuth for the header-present path of get_sdk_config (routes:54-60).
        if (operation == "get_profile_sdk_config" ||
            (operation == "get_sdk_config" && has_authorization_header)) {
            const auto publishable_key = ctx.claim(app::auth::kClaimPublishableKey);
            if (!publishable_key.has_value() || publishable_key->empty()) {
                send_api_error(res, send, unauthorized());
                return std::nullopt;
            }
        }
        return ctx;
    }

    // SdkAuthorization::decode (domain_models/sdk_auth.rs:134-236). BASE64_ENGINE is standard
    // base64 with padding (common_utils/consts.rs:124); the local codec is Base64Url, so the
    // alphabet is translated and padding dropped before decoding.
    [[nodiscard]] std::optional<SdkAuth> decode_sdk_authorization(std::string_view header) const
    {
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
            decoded = std::string{header};
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

    // ---------------------------------------------------------------- payment id / context ----------------------------------------------------------------

    struct PaymentIdResolution
    {
        std::string payment_id{};
        std::string client_secret{};
    };
    struct Resolution
    {
        std::optional<PaymentIdResolution> value{};
        std::optional<ApiError> error{};
    };

    // Upstream routes::get_payment_id_from_headers_or_payload (routes:19-49). The Authorization
    // header switches the whole flow to SDK auth; a decode failure is Unauthorized and a missing
    // payment_id is MissingRequiredField. Without the header, client_secret comes from the query
    // (never the body) and the id is derived by the payments helper.
    [[nodiscard]] Resolution get_payment_id_from_headers_or_payload(
        std::string_view auth_header,
        const std::optional<SdkAuth>& sdk_auth,
        const app::http::QueryParams& query
    ) const
    {
        Resolution out{};
        if (!auth_header.empty()) {
            if (!sdk_auth || !sdk_auth->decoded) {
                out.error = unauthorized();
                return out;
            }
            if (sdk_auth->payment_id.empty()) {
                out.error = missing_required_field("payment_id");
                return out;
            }
            out.value = PaymentIdResolution{sdk_auth->payment_id, sdk_auth->client_secret};
            return out;
        }

        const auto secret = query.get("client_secret");
        if (!secret || secret->empty()) {
            out.error = missing_required_field("client_secret");
            return out;
        }
        const auto payment_id = payment_id_from_client_secret(*secret);
        if (!payment_id || payment_id->empty()) {
            out.error = invalid_data_value("payment_id");
            return out;
        }
        out.value = PaymentIdResolution{*payment_id, *secret};
        return out;
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

    struct SdkPaymentContext
    {
        std::optional<app::store::payment_attempts::PaymentAttemptRecord> payment_attempt{};
        bool is_cit_transaction{false};
    };

    // Upstream core::get_payment_context (core:35-137). Attempt lookup, customer and mandate
    // state map onto AppRepository records; the local PaymentIntentRecord has no
    // shipping/billing address ids or setup_future_usage column, and no address store exists,
    // so is_cit_transaction uses the mandate-present branch of upstream's OR expression.
    [[nodiscard]] SdkPaymentContext get_payment_context(
        std::string_view payment_id,
        const app::store::payments::PaymentIntentRecord& payment
    ) const
    {
        SdkPaymentContext ctx{};
        ctx.payment_attempt = find_payment_attempt(payment_id);
        const bool has_customer = !payment.get_customer_id().empty() &&
            m_context->repository().find_customer_by_string(payment.get_customer_id()).has_value();
        const bool has_mandate = has_customer &&
            !m_context->repository().find_mandates_by_customer_id(payment.get_customer_id()).empty();
        ctx.is_cit_transaction = has_mandate;
        return ctx;
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
    list_enabled_merchant_connector_accounts(
        std::string_view merchant_id, std::string_view profile_id
    ) const
    {
        std::vector<app::store::merchant_connectors::MerchantConnectorRecord> result{};
        for (const auto& mca : m_context->repository().list_merchant_connectors()) {
            if (mca.get_disabled() || (!mca.get_status().empty() && mca.get_status() != "active")) {
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

    // ---------------------------------------------------------------- json helpers ----------------------------------------------------------------

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
        const std::optional<serde::Value>& object, std::string_view key
    ) const
    {
        if (!object) {
            return {};
        }
        return json_string_field(*object, key);
    }

    [[nodiscard]] std::string json_string_field(
        const serde::Value& value, std::string_view key
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
        const serde::Value& value, std::string_view key
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

    // ---------------------------------------------------------------- PM context / translation ----------------------------------------------------------------

    using PmKey = std::pair<std::string, std::string>;
    using ConnectorList = std::vector<std::string>;

    // Upstream MerchantEnabledPmsContext (core/payment_methods/cards.rs). The five maps mirror
    // the upstream fields; required_fields/pmt_to_auth_connector/installment/wallet/sdk_next_action
    // are empty defaults in both handlers at these call sites (core:274-289).
    struct MerchantEnabledPmsContext
    {
        std::map<PmKey, std::map<std::string, ConnectorList>> payment_experiences{};
        std::map<PmKey, std::map<std::string, ConnectorList>> card_networks{};
        std::map<std::string, ConnectorList> banks{};
        std::map<std::string, ConnectorList> bank_debits{};
        std::map<std::string, ConnectorList> bank_transfers{};

        // Upstream get_eligible_connectors(): every connector referenced by the context.
        [[nodiscard]] ConnectorList get_eligible_connectors() const
        {
            ConnectorList result{};
            const auto add = [&result](const ConnectorList& connectors) {
                for (const auto& connector : connectors) {
                    if (std::find(result.begin(), result.end(), connector) == result.end()) {
                        result.push_back(connector);
                    }
                }
            };
            for (const auto& [key, pe_map] : payment_experiences) {
                (void)key;
                for (const auto& [experience, connectors] : pe_map) {
                    (void)experience;
                    add(connectors);
                }
            }
            for (const auto& [key, network_map] : card_networks) {
                (void)key;
                for (const auto& [network, connectors] : network_map) {
                    (void)network;
                    add(connectors);
                }
            }
            for (const auto& [pmt, connectors] : banks) { (void)pmt; add(connectors); }
            for (const auto& [pmt, connectors] : bank_debits) { (void)pmt; add(connectors); }
            for (const auto& [pmt, connectors] : bank_transfers) { (void)pmt; add(connectors); }
            return result;
        }
    };

    static void add_connector(ConnectorList& connectors, std::string_view connector)
    {
        if (connector.empty() ||
            std::find(connectors.begin(), connectors.end(), connector) != connectors.end()) {
            return;
        }
        connectors.emplace_back(connector);
    }

    // Upstream build_merchant_enabled_pms_context (core:172-289, 386-397). MCA
    // payment_methods_enabled JSON is consolidated into payment-experience / card-network /
    // bank maps keyed exactly like the upstream HashMaps.
    [[nodiscard]] MerchantEnabledPmsContext build_merchant_enabled_pms_context(
        const std::vector<app::store::merchant_connectors::MerchantConnectorRecord>& mcas
    ) const
    {
        MerchantEnabledPmsContext ctx{};
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
                    const PmKey key{payment_method, pmt};

                    const std::string experience = json_string_field(type_value, "payment_experience");
                    if (!experience.empty()) {
                        add_connector(ctx.payment_experiences[key][experience], connector);
                    }
                    for (const auto& network : json_array(type_value, "card_networks")) {
                        const std::string name = json_string(network);
                        if (!name.empty()) {
                            add_connector(ctx.card_networks[key][name], connector);
                        }
                    }
                    if (payment_method == "bank_redirect") {
                        add_connector(ctx.banks[pmt], connector);
                    } else if (payment_method == "bank_debit") {
                        add_connector(ctx.bank_debits[pmt], connector);
                    } else if (payment_method == "bank_transfer") {
                        add_connector(ctx.bank_transfers[pmt], connector);
                    }
                }
            }
        }
        return ctx;
    }

    struct SdkBankCode
    {
        std::vector<std::string> bank_names{};
        ConnectorList eligible_connectors{};
    };

    // Upstream get_banks(state, pmt, connectors) (core/payment_methods/cards.rs). No bank
    // catalogue is exposed by AppContext; each enabled connector contributes one deterministic
    // bank name. A real catalogue should replace this local substitute (shared contract request).
    [[nodiscard]] std::vector<SdkBankCode> get_banks(
        std::string_view /*payment_method_type*/, const ConnectorList& connectors
    ) const
    {
        std::vector<SdkBankCode> result{};
        for (const auto& connector : connectors) {
            result.push_back(SdkBankCode{{connector}, {connector}});
        }
        return result;
    }

    // Upstream format_criteria_value (core:542-562).
    [[nodiscard]] std::string format_criteria_value(
        std::optional<std::string> card_network,
        std::optional<std::string> bank_name,
        std::optional<std::string> payment_experience
    ) const
    {
        std::vector<std::string> parts{};
        if (card_network && !card_network->empty()) {
            parts.push_back(*card_network);
        }
        if (bank_name && !bank_name->empty()) {
            parts.push_back(*bank_name);
        }
        if (payment_experience && !payment_experience->empty()) {
            parts.push_back(*payment_experience);
        }
        if (parts.empty()) {
            return "default";
        }
        std::string out{};
        for (std::size_t i = 0; i < parts.size(); ++i) {
            if (i > 0) {
                out.push_back('/');
            }
            out += parts[i];
        }
        return out;
    }

    // Upstream translate_to_sdk_payment_methods (core:564-688). The criteria is fixed by the
    // first source that inserts a (payment_method, payment_method_type) key: experiences, then
    // card networks, then banks, bank debits and bank transfers. Types are sorted by
    // payment_method_type and methods by payment_method, exactly as upstream does.
    [[nodiscard]] std::vector<app::dto::superposition_sdk_config::SdkPaymentMethod>
    translate_to_sdk_payment_methods(const MerchantEnabledPmsContext& pms_ctx) const
    {
        using namespace app::dto::superposition_sdk_config;
        struct Rules
        {
            std::optional<PaymentMethodCriteria> criteria{};
            std::vector<SdkCriteriaRule> rules{};
        };
        std::map<PmKey, Rules> consolidated{};
        const auto entry_for = [&consolidated](PmKey key, std::optional<PaymentMethodCriteria> criteria) -> Rules& {
            auto [it, inserted] = consolidated.try_emplace(std::move(key));
            if (inserted) {
                it->second.criteria = criteria;
            }
            return it->second;
        };

        // 1. payment experiences.
        for (const auto& [key, pe_map] : pms_ctx.payment_experiences) {
            auto& rules = entry_for(key, PaymentMethodCriteria::PaymentExperience);
            for (const auto& [experience, connectors] : pe_map) {
                rules.rules.push_back(SdkCriteriaRule{
                    format_criteria_value(std::nullopt, std::nullopt, experience), connectors
                });
            }
        }
        // 2. card networks.
        for (const auto& [key, network_map] : pms_ctx.card_networks) {
            auto& rules = entry_for(key, PaymentMethodCriteria::CardNetwork);
            for (const auto& [network, connectors] : network_map) {
                rules.rules.push_back(SdkCriteriaRule{
                    format_criteria_value(network, std::nullopt, std::nullopt), connectors
                });
            }
        }
        // 3. banks (bank redirect).
        for (const auto& [pmt, connectors] : pms_ctx.banks) {
            auto& rules = entry_for(PmKey{"bank_redirect", pmt}, PaymentMethodCriteria::BankName);
            bool has_banks{false};
            for (const auto& bank_code : get_banks(pmt, connectors)) {
                for (const auto& bank_name : bank_code.bank_names) {
                    has_banks = true;
                    rules.rules.push_back(SdkCriteriaRule{
                        format_criteria_value(std::nullopt, bank_name, std::nullopt),
                        bank_code.eligible_connectors
                    });
                }
            }
            if (!has_banks) {
                rules.criteria = std::nullopt;
                rules.rules.push_back(SdkCriteriaRule{
                    format_criteria_value(std::nullopt, std::nullopt, std::nullopt), connectors
                });
            }
        }
        // 4. bank debits.
        for (const auto& [pmt, connectors] : pms_ctx.bank_debits) {
            auto& rules = entry_for(PmKey{"bank_debit", pmt}, std::nullopt);
            rules.rules.push_back(SdkCriteriaRule{
                format_criteria_value(std::nullopt, std::nullopt, std::nullopt), connectors
            });
        }
        // 5. bank transfers.
        for (const auto& [pmt, connectors] : pms_ctx.bank_transfers) {
            auto& rules = entry_for(PmKey{"bank_transfer", pmt}, std::nullopt);
            rules.rules.push_back(SdkCriteriaRule{
                format_criteria_value(std::nullopt, std::nullopt, std::nullopt), connectors
            });
        }

        std::map<std::string, std::vector<SdkPaymentMethodType>> methods_map{};
        for (const auto& [key, rules] : consolidated) {
            if (rules.rules.empty()) {
                continue;
            }
            SdkPaymentMethodType type{};
            PaymentMethodTypeValue type_value{};
            type_value.set_value(key.second);
            type.set_payment_method_type(std::move(type_value));
            type.set_payment_method_criteria(rules.criteria);
            type.set_criteria_rules(rules.rules);
            methods_map[key.first].push_back(std::move(type));
        }

        std::vector<SdkPaymentMethod> result{};
        for (auto& [payment_method, types] : methods_map) {
            std::sort(types.begin(), types.end(), [](const auto& lhs, const auto& rhs) {
                return lhs.get_payment_method_type().get_value() <
                       rhs.get_payment_method_type().get_value();
            });
            SdkPaymentMethod method{};
            PaymentMethodValue method_value{};
            method_value.set_value(payment_method);
            method.set_payment_method(std::move(method_value));
            method.set_payment_method_types(std::move(types));
            result.push_back(std::move(method));
        }
        return result;
    }

    // ---------------------------------------------------------------- dimension filter / superposition ----------------------------------------------------------------

    [[nodiscard]] std::string provider_merchant_id(interfaces::io::IRequest& req) const
    {
        return app::http::HeaderReader{}.get(req, "x-provider-merchant-id");
    }

    [[nodiscard]] std::string organization_id(
        std::string_view merchant_id, const app::auth::AuthContext& auth_ctx
    ) const
    {
        if (!auth_ctx.org_id().empty()) {
            return auth_ctx.org_id();
        }
        const auto merchant = m_context->repository().find_merchant_account_by_string(merchant_id);
        return merchant ? merchant->get_organization_id() : std::string{};
    }

    [[nodiscard]] std::vector<std::string> enabled_connectors(
        const std::vector<app::store::merchant_connectors::MerchantConnectorRecord>& mcas
    ) const
    {
        ConnectorList result{};
        for (const auto& mca : mcas) {
            add_connector(result, mca.get_connector_name());
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    struct DimensionFilter
    {
        serde::Value value{};
        std::string key{};
    };

    // Upstream dimension filter (core:291-322, 399-433). [DYNAMIC_FIELDS, CHECKOUT_SDK] is
    // represented by the namespace/key pair passed to SuperpositionClient::resolve below.
    [[nodiscard]] DimensionFilter build_dimension_filter(
        std::string_view profile_id,
        std::string_view processor_merchant_id,
        std::string_view provider_merchant,
        std::string_view organization,
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

        const std::string json = std::format(
            R"({{"profile_id":{},"processor_merchant_id":{},"provider_merchant_id":{},"organization_id":{},"connector":{}}})",
            json_quote(profile_id),
            json_quote(processor_merchant_id),
            json_quote(provider_merchant),
            json_quote(organization),
            connectors_json
        );
        DimensionFilter filter{};
        auto decoded = parse_json(json);
        filter.value = decoded ? std::move(*decoded) : serde::Value{};
        // get_cached_config keys the superposition lookup by the dimension filter context; the
        // deterministic scoped hash is the local in-process equivalent.
        filter.key = app::crypto::KeyedHasher{"superposition_sdk_config"}.hash(json);
        return filter;
    }

    // Local substitute for the superposition service's eligibility dimension: the in-process
    // client scopes a config by a single context string, so CIT state is appended to the hash.
    [[nodiscard]] std::string filter_key_with_cit(
        std::string_view dimension_key, bool is_cit_transaction
    ) const
    {
        return is_cit_transaction ? std::string{dimension_key} + ":cit" : std::string{dimension_key};
    }

    // Upstream superposition_service.get_cached_config([DYNAMIC_FIELDS, CHECKOUT_SDK], filter)
    // (core:324-336, 435-447). SuperpositionClient is the repository-backed local service.
    // The stored document is interpreted as superposition_types::Config
    // (docs.rs 0.119.0: contexts, overrides, default_configs, dimensions); a bare config object
    // is placed in default_configs with the other three fields at their upstream Default values.
    [[nodiscard]] app::dto::superposition_sdk_config::Config resolve_raw_config(
        std::string_view dimension_key
    ) const
    {
        using Config = app::dto::superposition_sdk_config::Config;
        Config config{};
        const auto record = m_context->superposition().resolve(
            "dynamic_fields", "checkout_sdk", dimension_key
        );
        const std::string stored = (record && !record->get_value().empty())
            ? record->get_value()
            : std::string{R"({})"};
        const auto decoded = parse_json(stored);
        if (!decoded) {
            return config;
        }

        const auto field = [&decoded](std::string_view key) -> std::optional<serde::Value> {
            if (const auto* object = std::get_if<serde::Value::Object>(&decoded->get())) {
                const auto found = object->get(std::string{key});
                if (found && !std::holds_alternative<std::nullopt_t>(found->get())) {
                    return found;
                }
            }
            return std::nullopt;
        };

        if (const auto contexts = field("contexts")) {
            config.set_contexts(*contexts);
        }
        if (const auto overrides = field("overrides")) {
            config.set_overrides(*overrides);
        }
        if (const auto dimensions = field("dimensions")) {
            config.set_dimensions(*dimensions);
        }
        if (const auto defaults = field("default_configs")) {
            config.set_default_configs(*defaults);
        } else {
            config.set_default_configs(*decoded);
        }
        return config;
    }

    // ---------------------------------------------------------------- account config ----------------------------------------------------------------

    // Upstream get_feature_config (core/utils.rs) and get_should_perform_sdk_vaulting
    // (core/payment_methods/utils.rs), resolved from the local feature-config store.
    [[nodiscard]] bool feature_flag(std::string_view config_namespace, std::string_view key, bool fallback) const
    {
        const auto record = m_context->superposition().get_config(config_namespace, key);
        if (!record) {
            return fallback;
        }
        return record->get_value() == "true" || record->get_value() == "1";
    }

    [[nodiscard]] bool should_perform_sdk_vaulting(std::string_view organization_id) const
    {
        return feature_flag("feature_config", std::string{organization_id} + ".should_perform_sdk_vaulting", true);
    }

    [[nodiscard]] bool is_payment_method_modular_allowed(std::string_view merchant_id) const
    {
        return feature_flag("feature_config", std::string{merchant_id} + ".is_payment_method_modular_allowed", true);
    }

    // Upstream resolve_vaulting_action (core:531-540).
    [[nodiscard]] app::dto::superposition_sdk_config::VaultingAction resolve_vaulting_action(
        bool should_call_modular, bool should_perform_sdk_vaulting_flag
    ) const
    {
        using VaultingAction = app::dto::superposition_sdk_config::VaultingAction;
        return should_call_modular && should_perform_sdk_vaulting_flag
            ? VaultingAction::Tokenize
            : VaultingAction::Skip;
    }

    // Upstream build_profile_account_config (core:500-523). The local ProfileRecord has no
    // wallet-collection columns, so the four wallet flags are read from the profile-scoped
    // superposition config (unwrap_or(false) in both implementations).
    [[nodiscard]] app::dto::superposition_sdk_config::ProfileAccountConfig build_profile_account_config(
        std::string_view profile_id, bool modular, bool vaulting
    ) const
    {
        using namespace app::dto::superposition_sdk_config;
        const auto profile_flag = [this, profile_id](std::string_view flag) {
            const std::string key{std::string{profile_id} + "." + std::string{flag}};
            return feature_flag("sdk_account_config", key, false);
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
        profile.set_vaulting_action(resolve_vaulting_action(modular, vaulting));
        return profile;
    }

    // Upstream build_account_config (core:465-497).
    [[nodiscard]] app::dto::superposition_sdk_config::AccountConfig build_account_config(
        const app::store::profiles::ProfileRecord& /*profile*/,
        std::string_view profile_id,
        std::string_view merchant_id,
        const app::auth::AuthContext& auth_ctx
    ) const
    {
        using namespace app::dto::superposition_sdk_config;
        std::string org_id{};
        if (!auth_ctx.org_id().empty()) {
            org_id = auth_ctx.org_id();
        } else {
            const auto merchant = m_context->repository().find_merchant_account_by_string(merchant_id);
            if (merchant) {
                org_id = merchant->get_organization_id();
            }
        }
        const bool modular = is_payment_method_modular_allowed(merchant_id);
        const bool vaulting = should_perform_sdk_vaulting(org_id);
        AccountConfig account_config{};
        account_config.set_profile(build_profile_account_config(profile_id, modular, vaulting));
        return account_config;
    }

    // ---------------------------------------------------------------- response helpers ----------------------------------------------------------------

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error,
        std::optional<interfaces::io::types::Status> override_status = std::nullopt
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(override_status.value_or(core::error::AppErrorStatus{}(error.get_kind())));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::superposition_sdk_config
