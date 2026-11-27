export module app_routing:routing_runtime_routing_link_config;

import std;
import core_error;
import core_serde;
import app_context;
import core_behavior;

import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class RoutingLinkConfigRuntime
{
public:
    explicit RoutingLinkConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> routing_link_config(
        const std::optional<::app::model::routing::RoutingLinkWrapper>& link,
        const std::optional<::app::model::routing::RoutingActivatePayload>& activate,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        std::string algorithm = ctx_algorithm(ctx);
        std::string payload_txn{};

        if (link || activate) {
            if (link) {
                const auto& named = link->get_algorithm_id().get_routing_algorithm_id().get_value();
                if (!named.empty()) {
                    algorithm = named;
                }
                if (!link->get_profile_id().get_value().empty()) {
                    if (const auto validation_error = m_support.profile_validation_error(
                            ctx_auth_profile(ctx),
                            link->get_profile_id().get_value()
                        )) {
                        return std::unexpected{m_support.error(
                            core::error::AppErrorKind::PRECONDITION_FAILED,
                            *validation_error
                        )};
                    }
                }
            }
            if (activate && activate->get_transaction_type()) {
                payload_txn = activate->get_transaction_type()->get_value();
            }
        }
        if (algorithm.empty()) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found")
            };
        }

        const auto existing = m_support.find_config_by_algorithm(algorithm);
        if (!existing) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found")
            };
        }

        const auto txn = payload_txn.empty() ? ctx_transaction_type(ctx) : payload_txn;
        const auto record_txn = m_support.transaction_type_of(existing->get_config_json());
        if (record_txn != txn) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                std::format("Cannot use {}'s routing algorithm for {} operation", record_txn, txn)
            )};
        }

        auto profile =
            existing->get_profile_id().empty() ? ctx.profile_id : existing->get_profile_id();
        if (const auto active = m_support.active_id_for(ctx_query(ctx, "active_id"), profile, txn);
            active && *active == algorithm) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Algorithm is already active"
            )};
        }
        if (const auto validation_error =
                m_support.profile_validation_error(ctx_auth_profile(ctx), profile)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error)
            };
        }

        m_support.set_active_id(profile, txn, algorithm);
        m_support.track("routing_link_config");
        return ResponseDocument::from(m_support.dictionary_record(*existing));
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
