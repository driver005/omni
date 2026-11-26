export module app_payment:payouts_runtime_list;

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

class PayoutsListRuntime
{
public:
    explicit PayoutsListRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: list ─────────────────────────
    [[nodiscard]] std::expected<PayoutsListResult, core::error::AppError>
    list(const PayoutsListInput& input) const
    {
        m_context.metrics().record_flow("payout_list");
        m_context.metrics().counter("payouts_list_total").increment();

        auto response = m_support.list_response(
            m_support.list_filters_from_constraints(input.constraints),
            "", // merchant scope passes None profile_id_list (payouts.rs:354)
            input.range
        );
        return PayoutsListResult{response};
    }

private:
    ::app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
