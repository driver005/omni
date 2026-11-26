export module app_payment:payouts_runtime_get_payouts_aggregates_profile;

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

class PayoutsGetPayoutsAggregatesProfileRuntime
{
public:
    explicit PayoutsGetPayoutsAggregatesProfileRuntime(::app::context::AppContext& context) noexcept
        :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: get_payouts_aggregates_profile ─────────────────────────
    [[nodiscard]] std::expected<PayoutsAggregatesProfileResult, core::error::AppError>
    get_payouts_aggregates_profile(const PayoutsAggregatesProfileInput& input) const
    {
        m_context.metrics().record_flow("get_payouts_aggregates_profile");
        m_context.metrics().counter("payouts_aggregates_profile_total").increment();
        if (const auto invalid = m_support.require_time_range(input.range)) {
            return std::unexpected{*invalid};
        }

        auto response =
            m_support.aggregate_response(m_support.scoped_records(input.profile_id, input.range));
        return PayoutsAggregatesProfileResult{response};
    }

private:
    ::app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
