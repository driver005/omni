export module app_user:admin_input;

import std;
import interfaces;
import core_behavior;
import core_error;
import app_context;
import app_auth;
import app_http;
import app_dto;
import app_model;

export namespace app::handlers::admin {

/**
 * @brief Parse/auth/path/query support for the admin handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime. Every check mirrors the original
 * AdminRouteHandlers private helpers (authorize_request / path_part / path_has /
 * size_query), returning the error instead of writing it so the handler owns
 * the single response-writing path for every route.
 */
class AdminInputSupport
{
public:
    explicit AdminInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto context = ::app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(context, rule);
    }

    [[nodiscard]] bool path_has(interfaces::io::IRequest& req, std::string_view expected) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i{0}; i < segments.count(); ++i) {
            const auto part = segments.at(i);
            if (part.has_value() && *part == expected) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::size_t
    size_query(interfaces::io::IRequest& req, std::string_view key, std::size_t fallback) const
    {
        const auto raw = ::app::http::RequestQuery{}(req).get(key);
        if (!raw.has_value() || raw->empty()) {
            return fallback;
        }
        std::size_t value{fallback};
        const auto result = std::from_chars(raw->data(), raw->data() + raw->size(), value);
        return result.ec == std::errc{} ? value : fallback;
    }

private:
    ::app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old handler passed to its core call.

struct OrganizationCreateInput
{
    ::app::model::organization::OrganizationCreateRequest request{};
};

struct OrganizationUpdateInput
{
    std::string organization_id{};
    ::app::model::organization::OrganizationUpdateRequest request{};
};

struct ConvertOrganizationToPlatformInput
{
    std::string organization_id{};
    ::app::model::organization::ConvertOrganizationToPlatformRequest request{};
};

struct MerchantAccountCreateInput
{
    ::app::dto::admin::MerchantAccountCreate request{};
    std::string organization_id{};
};

struct UpdateMerchantAccountInput
{
    std::string merchant_id{};
    ::app::dto::admin::MerchantAccountUpdate request{};
};

struct ConnectorCreateInput
{
    std::string merchant_id{};
    ::app::dto::admin::MerchantConnectorCreate request{};
};

struct ConnectorUpdateInput
{
    std::string connector_id{};
    ::app::dto::admin::MerchantConnectorUpdate request{};
};

struct MerchantAccountToggleKvInput
{
    std::string merchant_id{};
    bool kv_enabled{};
};

struct MerchantAccountToggleAllKvInput
{
    bool kv_enabled{};
};

struct MerchantAccountTransferKeysInput
{
    std::size_t offset{};
    std::size_t limit{};
};

struct ConnectorWebhookRegisterInput
{
    std::string merchant_id{};
    std::string connector_id{};
    ::app::dto::admin::WebhookDetailsRequest request{};
};

struct OrganizationRetrieveInput
{
    std::string organization_id{};
};

struct RetrieveMerchantAccountInput
{
    std::string merchant_id{};
};

struct MerchantAccountListInput
{
    std::string organization_id{};
    std::string merchant_filter{};
    std::size_t offset{};
    std::size_t limit{};
};

struct DeleteMerchantAccountInput
{
    std::string merchant_id{};
};

struct ConnectorRetrieveInput
{
    std::string merchant_id{};
    std::string connector_id{};
};

struct ConnectorListInput
{
    std::string merchant_id{};
    std::string profile_id_filter{};
    bool for_profile{};
};

struct ConnectorListProfileInput
{
    std::string merchant_id{};
};

struct ConnectorDeleteInput
{
    std::string merchant_id{};
    std::string connector_id{};
};

struct MerchantAccountKvStatusInput
{
    std::string merchant_id{};
};

struct RetrieveConnectorWebhookInput
{
    std::string merchant_id{};
    std::string connector_id{};
};

/**
 * @brief Domain authorization policy for Admin.
 *
 * The route operation string selects the registered auth rule; the scope is
 * resolved from the request once, for every handler in the domain.
 */
class AdminPolicy
{
public:
    explicit AdminPolicy(AdminInputSupport& input) noexcept :
        m_input{input}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const ::core::behavior::AuthSpec&
    ) const
    {
        return m_input.authorize(req, operation);
    }

    [[nodiscard]] ::core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        ::core::behavior::Scope out{};
        out.merchant_id = std::string{::app::http::merchant_id_from_header{}(req)};
        if (out.merchant_id.empty()) {
            out.merchant_id = auth.merchant_id();
        }
        out.org_id = std::string{::app::http::org_id_from_header{}(req)};
        if (out.org_id.empty()) {
            out.org_id = auth.org_id();
        }
        out.profile_id = auth.profile_id();
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        if (connected.has_value()) {
            out.connected_merchant_id = std::string{*connected};
        }
        return out;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const ::core::behavior::Scope&) const
    {
        return std::nullopt;
    }

    /** @brief From-end segment index, matching the legacy path_part helper. */
    [[nodiscard]] static std::size_t path_index(interfaces::io::IRequest& req, std::size_t from_end)
    {
        const ::app::http::PathSegments segments{::app::http::RequestPath{}(req)};
        if (from_end == 0 || from_end > segments.count()) {
            return std::numeric_limits<std::size_t>::max();
        }
        return segments.count() - from_end;
    }

private:
    AdminInputSupport& m_input;
};

using AdminRequestSupport = ::core::behavior::RequestSupport<AdminPolicy>;

} // namespace app::handlers::admin
