export module app_payment:payouts_runtime_list_available_filters_for_merchant;

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

class PayoutsListAvailableFiltersForMerchantRuntime
{
public:
    explicit PayoutsListAvailableFiltersForMerchantRuntime(
        ::app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: list_available_filters_for_merchant ─────────────────────────
    [[nodiscard]] std::expected<PayoutsListAvailableFiltersForMerchantResult, core::error::AppError>
    list_available_filters_for_merchant(
        const PayoutsListAvailableFiltersForMerchantInput& input
    ) const
    {
        // Upstream input is `web::Json<TimeRange>`, not a filters object (`payouts.rs:497`).
        const auto range = m_support.time_range_from_body(input.body);
        if (const auto invalid = m_support.require_time_range(range)) {
            return std::unexpected{*invalid};
        }
        m_context.metrics().record_flow("payout_available_filters_merchant");
        m_context.metrics().counter("payouts_available_filters_merchant_total").increment();

        const auto filters = m_support.available_filters(m_support.scoped_records("", range));
        return PayoutsListAvailableFiltersForMerchantResult{filters};
    }

private:
    ::app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
