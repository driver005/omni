export module app_payment:payouts_runtime_get_payouts_aggregates;

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

import :payouts_input;
import :payouts_result;
import :payouts_runtime_common;

export namespace app::handlers::payouts {

class PayoutsGetPayoutsAggregatesRuntime
{
public:
    explicit PayoutsGetPayoutsAggregatesRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: get_payouts_aggregates ─────────────────────────
    [[nodiscard]] std::
        expected<::app::model::payouts::PayoutsAggregateResponse, core::error::AppError>
        get_payouts_aggregates(const PayoutsAggregatesInput& input) const
    {
        // Upstream input is `web::Query<TimeRange>` (`payouts.rs:708`); no body is read.
        m_context.metrics().record_flow("get_payouts_aggregates");
        m_context.metrics().counter("payouts_aggregates_total").increment();
        if (const auto invalid = m_support.require_time_range(input.range)) {
            return std::unexpected{*invalid};
        }

        auto response = m_support.aggregate_response(m_support.scoped_records("", input.range));
        return ::app::model::payouts::PayoutsAggregateResponse{response};
    }

private:
    ::app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
