export module app_routing:routing_runtime_routing_retrieve_config;

import std;
import core_error;
import core_serde;
import app_context;
import app_model;

import :routing_input;
import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class RoutingRetrieveConfigRuntime
{
public:
    explicit RoutingRetrieveConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    routing_retrieve_config(const RoutingInput& input) const
    {
        const auto existing = m_support.find_config_by_algorithm(input.algorithm_id);
        if (!existing) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found")
            };
        }
        if (const auto validation_error = m_support.profile_validation_error(
                input.auth_profile,
                existing->get_profile_id()
            )) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error)
            };
        }

        // MerchantRoutingAlgorithm (api_models/src/routing.rs:323).
        ::app::model::routing::MerchantRoutingAlgorithm response;
        response.set_id(
            m_support.wrap<::app::model::routing::RoutingIdValue>(existing->get_config_id())
        );
        response.set_profile_id(
            m_support.wrap<::app::model::routing::ProfileIdValue>(existing->get_profile_id())
        );
        response.set_name(existing->get_name());
        response.set_description(existing->get_description());
        response.set_algorithm(::app::model::routing::RoutingAlgorithmWrapper::Static);
        response.set_created_at(m_support.now_unix());
        response.set_modified_at(m_support.now_unix());
        response.set_algorithm_for(m_support.wrap<::app::model::routing::TransactionTypeValue>(
            m_support.transaction_type_of(existing->get_config_json())
        ));
        m_support.track("routing_retrieve_config");
        return ResponseDocument::from(response);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
