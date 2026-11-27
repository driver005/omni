export module app_payment:disputes_runtime_get_platform_disputes_filters;

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

class DisputesGetPlatformDisputesFiltersRuntime
{
public:
    explicit DisputesGetPlatformDisputesFiltersRuntime(::app::context::AppContext& context) noexcept
        :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::disputes::PlatformDisputeListFilters>
    get_platform_disputes_filters(const core::behavior::CallContext& ctx) const
    {
        const auto input = GetPlatformDisputesFiltersData{ctx};
        m_context.metrics().record_flow("dispute_platform_filters");
        auto result = m_support.get_platform_disputes_filters(input.scope);
        if (!result) {
            return std::unexpected{result.error()};
        }
        return ::app::model::disputes::PlatformDisputeListFilters{*result};
    }

private:
    ::app::context::AppContext& m_context;
    DisputesRuntimeSupport m_support;
};

} // namespace app::handlers::disputes
