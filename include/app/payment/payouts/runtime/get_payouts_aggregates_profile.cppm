export module app_payment:payouts_runtime_get_payouts_aggregates_profile;
import core_behavior;

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
    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutsAggregateResponse>
    get_payouts_aggregates_profile(const core::behavior::CallContext& ctx) const
    {
        const auto input = PayoutsAggregatesProfileData{ctx};
        m_context.logger().record_flow("get_payouts_aggregates_profile");
        m_context.logger().counter("payouts_aggregates_profile_total").increment();
        if (const auto invalid = m_support.require_time_range(input.range)) {
            return std::unexpected{*invalid};
        }

        auto response =
            m_support.aggregate_response(m_support.scoped_records(input.profile_id, input.range));
        return ::app::model::payouts::PayoutsAggregateResponse{response};
    }

private:
    ::app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
