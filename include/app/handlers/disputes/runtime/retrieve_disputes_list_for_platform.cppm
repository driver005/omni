export module app_handlers_disputes:runtime_retrieve_disputes_list_for_platform;

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

class DisputesRetrieveDisputesListForPlatformRuntime
{
public:
    explicit DisputesRetrieveDisputesListForPlatformRuntime(
        app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RetrieveDisputesListForPlatformResult, Error>
    retrieve_disputes_list_for_platform(const RetrieveDisputesListForPlatformInput& input) const
    {
        m_context.metrics().record_flow("dispute_platform_list");
        const auto& scope = input.scope;
        const auto& constraints = input.constraints;
        // core/disputes.rs:299-305 — platform-account guard is enforced by scope_error.
        // core/disputes.rs:307,313 — provider scoping plus optional processor_merchant_id.
        const std::string processor = constraints.get_processor_merchant_id().has_value()
                                          ? constraints.get_processor_merchant_id()->get_value()
                                          : std::string{};
        const auto records = m_support.find_disputes_by_constraints(
            scope,
            m_support.platform_as_list_constraints(constraints),
            std::nullopt,
            true,
            true,
            processor
        );
        // core/disputes.rs:318-322 — total_count ignores limit/offset.
        const std::int64_t total_count =
            static_cast<std::int64_t>(m_support
                                          .find_disputes_by_constraints(
                                              scope,
                                              m_support.platform_as_list_constraints(constraints),
                                              std::nullopt,
                                              true,
                                              false,
                                              processor
                                          )
                                          .size());

        app::model::disputes::PlatformDisputeListResponse response{};
        response.set_count(records.size());
        response.set_total_count(total_count);
        std::vector<app::model::disputes::PlatformDisputeListItem> items{};
        items.reserve(records.size());
        for (const auto& record: records) {
            items.push_back(m_support.to_platform_item(record));
        }
        response.set_data(std::move(items));
        return RetrieveDisputesListForPlatformResult{response};
    }

private:
    app::context::AppContext& m_context;
    DisputesRuntimeSupport m_support;
};

} // namespace app::handlers::disputes
