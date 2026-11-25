export module app_handlers_payouts:runtime_list_profile;

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

class PayoutsListProfileRuntime
{
public:
    explicit PayoutsListProfileRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: list_profile ─────────────────────────
    [[nodiscard]] std::expected<PayoutsListProfileResult, core::error::AppError>
    list_profile(const PayoutsListProfileInput& input) const
    {
        m_context.metrics().record_flow("payout_list_profile");
        m_context.metrics().counter("payouts_list_profile_total").increment();

        auto response = m_support.list_response(
            m_support.list_filters_from_constraints(input.constraints),
            input.profile_id,
            input.range
        );
        return PayoutsListProfileResult{response};
    }

private:
    app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
