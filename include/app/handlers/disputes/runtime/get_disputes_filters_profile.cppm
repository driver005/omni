export module app_handlers_disputes:runtime_get_disputes_filters_profile;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::disputes {

class DisputesGetDisputesFiltersProfileRuntime
{
public:
    explicit DisputesGetDisputesFiltersProfileRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetDisputesFiltersProfileResult, Error>
    get_disputes_filters_profile(const GetDisputesFiltersProfileInput& input) const
    {
        m_context.metrics().record_flow("dispute_profile_filters");
        auto result = m_support.get_filters_for_disputes(input.scope, input.profile_id_list);
        if (!result) {
            return std::unexpected{result.error()};
        }
        return GetDisputesFiltersProfileResult{*result};
    }

private:
    app::context::AppContext& m_context;
    DisputesRuntimeSupport m_support;
};

} // namespace app::handlers::disputes
