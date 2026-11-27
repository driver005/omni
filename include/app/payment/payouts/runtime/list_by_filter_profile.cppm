export module app_payment:payouts_runtime_list_by_filter_profile;

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

class PayoutsListByFilterProfileRuntime
{
public:
    explicit PayoutsListByFilterProfileRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: list_by_filter_profile ─────────────────────────
    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutListResponse>
        list_by_filter_profile(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = PayoutsListByFilterProfileData{dto, ctx};
        const auto filters = m_support.list_filters_from_dto(input.request, input.query);
        if (filters.limit > 20) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "limit should be in between 1 and 20"
            )};
        }
        m_context.metrics().record_flow("payout_list_by_filter_profile");
        m_context.metrics().counter("payouts_list_by_filter_profile_total").increment();

        auto response = m_support.list_response(
            filters,
            input.profile_id,
            m_support.time_range_from_body(input.body)
        );
        return ::app::model::payouts::PayoutListResponse{response};
    }

private:
    ::app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
