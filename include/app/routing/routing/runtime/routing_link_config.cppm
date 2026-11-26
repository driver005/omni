export module app_routing:routing_runtime_routing_link_config;

import std;
import core_error;
import core_serde;
import app_context;

import :routing_input;
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

    [[nodiscard]] std::expected<RoutingLinkConfigResult, core::error::AppError>
    routing_link_config(const RoutingInput& input) const
    {
        std::string algorithm = input.algorithm_id;
        std::string payload_txn{};

        if (!input.body.empty()) {
            if (input.link) {
                const auto& named =
                    input.link->get_algorithm_id().get_routing_algorithm_id().get_value();
                if (!named.empty()) {
                    algorithm = named;
                }
                if (!input.link->get_profile_id().get_value().empty()) {
                    if (const auto validation_error = m_support.profile_validation_error(
                            input.auth_profile,
                            input.link->get_profile_id().get_value()
                        )) {
                        return std::unexpected{m_support.error(
                            core::error::AppErrorKind::PRECONDITION_FAILED,
                            *validation_error
                        )};
                    }
                }
            }
            if (input.activate && input.activate->get_transaction_type()) {
                payload_txn = input.activate->get_transaction_type()->get_value();
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

        const auto txn = payload_txn.empty() ? input.txn : payload_txn;
        const auto record_txn = m_support.transaction_type_of(existing->get_config_json());
        if (record_txn != txn) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                std::format("Cannot use {}'s routing algorithm for {} operation", record_txn, txn)
            )};
        }

        auto profile =
            existing->get_profile_id().empty() ? input.profile_id : existing->get_profile_id();
        if (const auto active = m_support.active_id_for(input.active_id, profile, txn);
            active && *active == algorithm) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Algorithm is already active"
            )};
        }
        if (const auto validation_error =
                m_support.profile_validation_error(input.auth_profile, profile)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error)
            };
        }

        m_support.set_active_id(profile, txn, algorithm);
        m_support.track("routing_link_config");
        return RoutingLinkConfigResult{
            ResponseDocument::from(m_support.dictionary_record(*existing))
        };
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
