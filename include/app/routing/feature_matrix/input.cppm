export module app_routing:feature_matrix_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_auth;

import :policy;

export namespace app::handlers::feature_matrix {

// The only request data this GET route consumes is its optional JSON body: an
// object with an optional "connectors" array of snake_case connector names.
// Body decoding stays in the runtime so the auth + metrics side effects recorded
// by the original handler still run before any body validation error is produced.
struct FetchFeatureMatrixInput
{
    std::string body{};
};

/**
 * @brief Domain policy for the single NO_AUTH feature-matrix route.
 *
 * Keeps the original rule-registry lookup so the pass-through error semantics
 * (and operation string) do not change.
 */
class FeatureMatrixPolicy
{
public:
    explicit FeatureMatrixPolicy(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const ::core::behavior::AuthSpec&
    ) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] ::core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        return ::app::routing::policy::scope_of(req);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const ::core::behavior::Scope& scope) const
    {
        return ::app::routing::policy::scope_error(scope);
    }

private:
    ::app::context::AppContext& m_context;
};

using FeatureMatrixRequestSupport = ::core::behavior::RequestSupport<FeatureMatrixPolicy>;

} // namespace app::handlers::feature_matrix
