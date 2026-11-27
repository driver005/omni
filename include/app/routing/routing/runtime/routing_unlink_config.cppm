export module app_routing:routing_runtime_routing_unlink_config;

import std;
import core_error;
import core_serde;
import app_context;
import core_behavior;

import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class RoutingUnlinkConfigRuntime
{
public:
    explicit RoutingUnlinkConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> routing_unlink_config(
        const std::optional<::app::model::routing::RoutingConfigRequest>& config_request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        auto profile = ctx.profile_id;
        if (ctx_is_v2(ctx)) {
            if (config_request && !config_request->get_profile_id().get_value().empty()) {
                profile = config_request->get_profile_id().get_value();
            }
        } else if (config_request && !config_request->get_profile_id().get_value().empty()) {
            profile = config_request->get_profile_id().get_value();
        }
        if (profile.empty()) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INVALID_REQUEST, "profile_id")
            };
        }
        if (const auto validation_error =
                m_support.profile_validation_error(ctx_auth_profile(ctx), profile)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error)
            };
        }

        const auto txn = ctx_transaction_type(ctx);
        const auto active = m_support.active_id_for(ctx_query(ctx, "active_id"), profile, txn);
        if (!active) {
            // "Algorithm is already inactive" (core:1941-1943 / :1760-1762).
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Algorithm is already inactive"
            )};
        }
        const auto record = m_support.find_config_by_algorithm(*active);
        if (!record) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found")
            };
        }
        m_support.set_active_id(profile, txn, std::string{});
        auto response = m_support.dictionary_record(*record);
        m_support.track("routing_unlink_config");
        return ResponseDocument::from(response);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
