export module app_handlers_admin:input;

import std;
import interfaces;
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
    explicit AdminInputSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(context, rule);
    }

    [[nodiscard]] std::string path_part(interfaces::io::IRequest& req, std::size_t from_end) const
    {
        const auto segments = app::http::RequestPath{}(req);
        if (from_end == 0 || from_end > segments.count()) {
            return {};
        }
        const auto part = segments.at(segments.count() - from_end);
        return part.has_value() ? std::string{*part} : std::string{};
    }

    [[nodiscard]] bool path_has(interfaces::io::IRequest& req, std::string_view expected) const
    {
        const auto segments = app::http::RequestPath{}(req);
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
        const auto raw = app::http::RequestQuery{}(req).get(key);
        if (!raw.has_value() || raw->empty()) {
            return fallback;
        }
        std::size_t value{fallback};
        const auto result = std::from_chars(raw->data(), raw->data() + raw->size(), value);
        return result.ec == std::errc{} ? value : fallback;
    }

private:
    app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old handler passed to its core call.

struct OrganizationCreateInput
{
    app::model::organization::OrganizationCreateRequest request{};
};

struct OrganizationUpdateInput
{
    std::string organization_id{};
    app::model::organization::OrganizationUpdateRequest request{};
};

struct ConvertOrganizationToPlatformInput
{
    std::string organization_id{};
    app::model::organization::ConvertOrganizationToPlatformRequest request{};
};

struct MerchantAccountCreateInput
{
    app::dto::admin::MerchantAccountCreate request{};
    std::string organization_id{};
};

struct UpdateMerchantAccountInput
{
    std::string merchant_id{};
    app::dto::admin::MerchantAccountUpdate request{};
};

struct ConnectorCreateInput
{
    std::string merchant_id{};
    app::dto::admin::MerchantConnectorCreate request{};
};

struct ConnectorUpdateInput
{
    std::string connector_id{};
    app::dto::admin::MerchantConnectorUpdate request{};
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
    app::dto::admin::WebhookDetailsRequest request{};
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

} // namespace app::handlers::admin
