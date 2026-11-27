export module app_payment:disputes_runtime_retrieve_disputes_list_profile;

import std;
import interfaces;
import core_router;
import core_serde;
import core_cache;
import core_database;
import core_error;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import app_metrics;
import app_store;
import app_connector;
import app_workflow;
import app_events;
import app_locking;
import app_services;

import :disputes_input;
import :disputes_result;
import :disputes_runtime_common;

export namespace app::handlers::disputes {

class DisputesRetrieveDisputesListProfileRuntime
{
public:
    explicit DisputesRetrieveDisputesListProfileRuntime(
        ::app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::vector<::app::model::disputes::DisputeResponse>, Error>
    retrieve_disputes_list_profile(const RetrieveDisputesListProfileInput& input) const
    {
        m_context.metrics().record_flow("dispute_profile_list");
        auto result =
            m_support.retrieve_disputes_list(input.scope, input.constraints, input.profile_id_list);
        if (!result) {
            return std::unexpected{result.error()};
        }
        return std::vector<::app::model::disputes::DisputeResponse>{std::move(*result)};
    }

private:
    ::app::context::AppContext& m_context;
    DisputesRuntimeSupport m_support;
};

} // namespace app::handlers::disputes
