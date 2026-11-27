export module app_routing:routing_runtime_routing_update_default_config;

import std;
import core_error;
import core_serde;
import app_context;
import app_model;
import core_behavior;

import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class RoutingUpdateDefaultConfigRuntime
{
public:
    explicit RoutingUpdateDefaultConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> routing_update_default_config(
        const Connectors& connectors,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        const auto profile = ctx.profile_id;
        if (ctx_is_v2(ctx)) {
            if (const auto validation_error =
                    m_support.profile_validation_error(ctx_auth_profile(ctx), profile)) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::PRECONDITION_FAILED,
                    *validation_error
                )};
            }
        }
        if (const auto validation_error =
                m_support.validate_default_connectors(profile, connectors)) {
            return std::unexpected{*validation_error};
        }
        m_support.store_default_config(ctx.merchant_id, profile, connectors);

        if (ctx_is_v2(ctx)) {
            ::app::model::routing::ProfileDefaultRoutingConfig response;
            response.set_profile_id(m_support.wrap<::app::model::routing::ProfileIdValue>(profile));
            response.set_connectors(connectors);
            m_support.track("routing_update_default_config");
            return ResponseDocument::from(response);
        }
        m_support.track("routing_update_default_config");
        return ResponseDocument::from(connectors);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
