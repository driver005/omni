export module app_handlers_gsm:input;

import std;
import interfaces;
import core_error;
import app_context;
import app_auth;
import app_dto;

export namespace app::handlers::gsm {

/**
 * @brief Auth extraction for the GSM handlers.
 *
 * The facade used a single admin extractor for all four routes (v1 and v2
 * scopes wire the same functions). Kept here so handler methods only perform
 * request -> Input conversion and response writing.
 */
class GsmInputSupport
{
public:
    explicit GsmInputSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize_request(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(context, rule);
    }

private:
    app::context::AppContext& m_context;
};

// Each input carries exactly what the old inline core block consumed.
struct CreateGsmRuleInput
{
    app::dto::gsm::GsmCreateRequest request{};
};

struct GetGsmRuleInput
{
    app::dto::gsm::GsmRetrieveRequest request{};
};

struct UpdateGsmRuleInput
{
    app::dto::gsm::GsmUpdateRequest request{};
};

struct DeleteGsmRuleInput
{
    app::dto::gsm::GsmDeleteRequest request{};
};

} // namespace app::handlers::gsm
