export module app_handlers_payouts:runtime_get_payout_filters;

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

class PayoutsGetFiltersRuntime
{
public:
    explicit PayoutsGetFiltersRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: get_payout_filters ─────────────────────────
    [[nodiscard]] std::expected<PayoutsGetFiltersResult, core::error::AppError>
    get_payout_filters(const PayoutsGetFiltersInput& input) const
    {
        m_context.metrics().record_flow("get_payout_filters");
        m_context.metrics().counter("payouts_get_filters_total").increment();

        return PayoutsGetFiltersResult{m_support.filters_v2_from_mcas("", input.merchant_id)};
    }

private:
    app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
