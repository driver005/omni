export module app_handlers_payouts:runtime_list_by_filter;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::payouts {

class PayoutsListByFilterRuntime
{
public:
    explicit PayoutsListByFilterRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: list_by_filter ─────────────────────────
    [[nodiscard]] std::expected<PayoutsListByFilterResult, core::error::AppError>
    list_by_filter(const PayoutsListByFilterInput& input) const
    {
        const auto filters = m_support.list_filters_from_dto(input.request, input.query);
        // `validate_payout_list_request_for_joins` (validator.rs:609-620, max limit 20).
        if (filters.limit > 20) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "limit should be in between 1 and 20"
            )};
        }
        m_context.metrics().record_flow("payout_list_by_filter");
        m_context.metrics().counter("payouts_list_by_filter_total").increment();

        auto response =
            m_support.list_response(filters, "", m_support.time_range_from_body(input.body));
        return PayoutsListByFilterResult{response};
    }

private:
    app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
