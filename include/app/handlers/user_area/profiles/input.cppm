export module app_user:profiles_input;

import std;
import interfaces;
import core_error;
import app_context;
import app_dto;
import app_auth;
import app_http;

export namespace app::handlers::profiles {

/**
 * @brief Parse/auth/path/query support for the profile handlers.
 *
 * Only request-derived work: no business logic and no response writing. Route-layer
 * validation runs here so the handler keeps the original error precedence.
 */
class ProfilesInputSupport
{
public:
    explicit ProfilesInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    using CreateInput = ::app::dto::admin::ProfileCreate;
    using UpdateInput = ::app::dto::admin::ProfileUpdate;

    // routes/profiles.rs:22-45 (create v1), :79-90 (create v2),
    // :235-259 (update v1), :314-325 (update v2) — WebhookDetailsRequest::validate
    // (api_models/admin.rs:1026-1052) and BusinessPaymentLinkConfig::validate
    // (api_models/admin.rs:3759-3795).
    [[nodiscard]] std::optional<core::error::AppError>
    validate_profile_create_request(const CreateInput& request) const
    {
        if (request.get_webhook_details()) {
            if (const auto error = validate_webhook_details(*request.get_webhook_details())) {
                return error;
            }
        }
        if (request.get_payment_link_config()) {
            if (const auto error =
                    validate_payment_link_config(*request.get_payment_link_config())) {
                return error;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    validate_profile_update_request(const UpdateInput& request) const
    {
        if (request.get_webhook_details()) {
            if (const auto error = validate_webhook_details(*request.get_webhook_details())) {
                return error;
            }
        }
        if (request.get_payment_link_config()) {
            if (const auto error =
                    validate_payment_link_config(*request.get_payment_link_config())) {
                return error;
            }
        }
        return std::nullopt;
    }

    // Per-operation rule from app/auth/registry.cppm; routes carry the
    // upstream route-vs-header merchant binding (profiles.rs:55-64,113-121,
    // 287-297,520-530,584-592).
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_profile_operation(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        auto auth = ::app::auth::AuthContextBuilder{}(req);
        const std::string route_merchant = merchant_from_request(req);

        // ApiKeyAuthWithMerchantIdFromRoute / JWTAuthMerchantFromRoute: when a
        // header merchant is present it must equal the route merchant. Connected
        // scope is honoured through the connected-merchant claim.
        if (!route_merchant.empty() && !auth.merchant_id().empty() &&
            auth.merchant_id() != route_merchant) {
            const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
            if (!connected || connected->empty() || *connected != route_merchant) {
                return make_error(
                    core::error::AppErrorKind::FORBIDDEN,
                    "merchant id header does not match route merchant"
                );
            }
        }
        if (!route_merchant.empty()) {
            if (!auth.merchant_id().empty()) {
                auth.set_merchant_id(route_merchant);
            } else if (auth.claim(::app::auth::kClaimConnectedMerchantId).has_value()) {
                auth.set_merchant_id(route_merchant);
            }
        }
        return m_context.authorizer().authorize_route(auth, rule);
    }

    [[nodiscard]] std::optional<std::vector<std::string>>
    profile_filter_from_auth(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (!auth.profile_id().empty()) {
            return std::vector<std::string>{auth.profile_id()};
        }
        return std::nullopt;
    }

    [[nodiscard]] bool is_v2(interfaces::io::IRequest& req) const
    {
        return ::app::http::RequestPath{}(req).at(0).value_or("") == "v2";
    }

    [[nodiscard]] std::string
    path_segment(interfaces::io::IRequest& req, std::size_t from_end = 0) const
    {
        const ::app::http::PathSegments segments{::app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        if (from_end >= count) {
            return {};
        }
        const auto value = segments.at(count - 1 - from_end);
        return value ? std::string{*value} : std::string{};
    }

    [[nodiscard]] std::string merchant_from_request(interfaces::io::IRequest& req) const
    {
        const ::app::http::PathSegments segments{::app::http::RequestPath{}(req)};
        if (segments.at(0).value_or("") == "v2") {
            if (segments.at(1).value_or("") == "merchant-accounts") {
                return std::string{segments.at(2).value_or("")};
            }
            return ::app::http::merchant_id_from_header{}(req);
        }
        if (segments.at(0).value_or("") == "profiles") {
            return ::app::http::merchant_id_from_header{}(req);
        }
        return std::string{segments.at(1).value_or("")};
    }

    [[nodiscard]] std::string
    profile_id_from_request(interfaces::io::IRequest& req, std::size_t from_end = 0) const
    {
        const ::app::http::PathSegments segments{::app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        if (from_end == 0 && count >= 3) {
            const auto parent = segments.at(count - 2).value_or("");
            if (parent == "profiles" || parent == "business_profile") {
                return std::string{segments.at(count - 1).value_or("")};
            }
            return {};
        }
        return path_segment(req, from_end);
    }

private:
    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    // api_models/admin.rs:1026-1052 — statuses are typed enums at the C++ DTO
    // boundary (IntentStatusValue/RefundStatusValue/...), so unknown values are
    // already rejected by deserialization; the URL scheme check is explicit.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_webhook_details(const ::app::dto::admin::WebhookDetailsRequest& details) const
    {
        if (details.get_webhook_url() && !details.get_webhook_url()->empty()) {
            if (!details.get_webhook_url()->starts_with("http://") &&
                !details.get_webhook_url()->starts_with("https://")) {
                return make_error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "webhook_url is not valid"
                );
            }
        }
        return std::nullopt;
    }

    // api_models/admin.rs:3759-3795 — strict host domain + wildcard allowed
    // domains + nested default/business-specific config validation.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_payment_link_config(const ::app::dto::admin::BusinessPaymentLinkConfig& config) const
    {
        if (config.get_domain_name() && !validate_strict_domain(*config.get_domain_name())) {
            return make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid host domain name received in payment_link_config"
            );
        }
        if (config.get_allowed_domains()) {
            for (const auto& domain: *config.get_allowed_domains()) {
                if (!validate_wildcard_domain(domain)) {
                    return make_error(
                        core::error::AppErrorKind::INVALID_REQUEST,
                        "Invalid allowed domain names received in payment_link_config"
                    );
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] bool validate_strict_domain(std::string_view domain) const
    {
        if (domain.empty() || domain.size() > 253) {
            return false;
        }
        std::size_t start{0};
        while (start < domain.size()) {
            const auto dot = domain.find('.', start);
            const auto label = domain.substr(
                start,
                dot == std::string_view::npos ? std::string_view::npos : dot - start
            );
            if (label.empty() || label.size() > 63 || label.front() == '-' || label.back() == '-') {
                return false;
            }
            for (const char c: label) {
                if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-') {
                    return false;
                }
            }
            if (dot == std::string_view::npos) {
                break;
            }
            start = dot + 1;
        }
        return true;
    }

    [[nodiscard]] bool validate_wildcard_domain(std::string_view domain) const
    {
        if (domain.starts_with("*.")) {
            domain.remove_prefix(2);
        }
        return validate_strict_domain(domain);
    }

    ::app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old handler passed into its core call.

struct ProfileCreateInput
{
    ::app::dto::admin::ProfileCreate request{};
    std::string merchant_id{};
    bool v2_route{};
};

struct ProfileRetrieveInput
{
    std::string profile_id{};
    std::string merchant_id{};
};

struct ProfileUpdateInput
{
    std::string profile_id{};
    std::string merchant_id{};
    ::app::dto::admin::ProfileUpdate request{};
};

struct ProfileDeleteInput
{
    std::string profile_id{};
    std::string merchant_id{};
};

struct ProfilesListInput
{
    std::string merchant_id{};
    std::optional<std::vector<std::string>> profile_id_list{};
};

struct ProfilesListAtProfileLevelInput
{
    std::string merchant_id{};
    std::optional<std::vector<std::string>> profile_id_list{};
};

struct ToggleConnectorAgnosticMitInput
{
    std::string merchant_id{};
    std::string profile_id{};
    ::app::dto::admin::ConnectorAgnosticMitChoice choice{};
};

struct ToggleExtendedCardInfoInput
{
    std::string merchant_id{};
    std::string profile_id{};
    ::app::dto::admin::ExtendedCardInfoChoice choice{};
};

} // namespace app::handlers::profiles
