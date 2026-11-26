export module app_store:health_input;

import std;
import interfaces;
import core_error;
import app_context;
import app_context_core;

export namespace app::handlers::health {

/**
 * @brief Auth support for the health handlers.
 *
 * Both routes are NO_AUTH in the shared registry; authorize_route is an exact
 * no-op for them, but the call (and its error path) is preserved verbatim.
 */
class HealthInputSupport
{
public:
    explicit HealthInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

private:
    ::app::context::AppContext& m_context;
};

// Neither route reads a body, path or query, so the Input structs carry no
// fields. They exist to keep the 3-job handler shape uniform.
struct HealthInput
{};

struct DeepHealthCheckInput
{};

} // namespace app::handlers::health
