export module app_auth:hypersense_runtime_token;

import std;
import core_behavior;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;
import app_services;

import :call_context;
import :hypersense_input;
import :hypersense_result;
import :hypersense_runtime_common;

export namespace app::handlers::hypersense {

class HypersenseTokenRuntime
{
public:
    explicit HypersenseTokenRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // routes/hypersense.rs:28-31 -> core/external_service_auth.rs:22-26
    // generate_external_token(state, user, ExternalServiceType::Hypersense).
    [[nodiscard]] core::behavior::Outcome<::app::dto::external_service_auth::ExternalTokenResponse>
    get_token(const core::behavior::CallContext& ctx) const
    {
        m_support.record_flow("HypersenseTokenRequest", LockActionNotApplicable{});
        m_context.logger().counter("hypersense.token_request").increment();

        // DashboardNoPermissionAuth: parse the dashboard JWT, check its blacklist
        // and tenant, then hand UserFromToken to the token generator. No named
        // permission is required.
        auto user = m_support.authenticate_dashboard(detail::bearer_token(ctx), ctx.merchant_id);
        if (!user) {
            return std::unexpected{user.error()};
        }

        const auto token = m_support.generate_external_token(*user);
        if (!token) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "InternalServerError"
            )};
        }

        using Response = ::app::dto::external_service_auth::ExternalTokenResponse;
        Response response{};
        response.set_token(*token);
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    HypersenseRuntimeSupport m_support;
};

} // namespace app::handlers::hypersense
