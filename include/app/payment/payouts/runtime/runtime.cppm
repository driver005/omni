export module app_payment:payouts_runtime;
import core_behavior;

import std;
import core_error;
import app_context;

import :payouts_input;
import :payouts_result;
import :payouts_runtime_common;
import :payouts_runtime_create;
import :payouts_runtime_retrieve;
import :payouts_runtime_update;
import :payouts_runtime_confirm;
import :payouts_runtime_cancel;
import :payouts_runtime_fulfill;
import :payouts_runtime_list;
import :payouts_runtime_list_profile;
import :payouts_runtime_list_by_filter;
import :payouts_runtime_list_by_filter_profile;
import :payouts_runtime_list_available_filters_for_merchant;
import :payouts_runtime_list_available_filters_for_profile;
import :payouts_runtime_get_payout_filters;
import :payouts_runtime_get_payout_filters_profile;
import :payouts_runtime_get_payouts_aggregates;
import :payouts_runtime_get_payouts_aggregates_profile;
import :payouts_runtime_manual_update;

export namespace app::handlers::payouts {

class PayoutsRuntime
{
public:
    explicit PayoutsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_retrieve{context},
        m_update{context},
        m_confirm{context},
        m_cancel{context},
        m_fulfill{context},
        m_list{context},
        m_list_profile{context},
        m_list_by_filter{context},
        m_list_by_filter_profile{context},
        m_list_available_filters_for_merchant{context},
        m_list_available_filters_for_profile{context},
        m_get_payout_filters{context},
        m_get_payout_filters_profile{context},
        m_get_payouts_aggregates{context},
        m_get_payouts_aggregates_profile{context},
        m_manual_update{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutCreateResponse>
    create(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_create.create(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutCreateResponse>
    retrieve(const core::behavior::CallContext& ctx) const
    {
        return m_retrieve.retrieve(ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutCreateResponse>
    update(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_update.update(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutCreateResponse>
    confirm(const core::behavior::CallContext& ctx) const
    {
        return m_confirm.confirm(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutCreateResponse>
    cancel(const core::behavior::CallContext& ctx) const
    {
        return m_cancel.cancel(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutCreateResponse>
    fulfill(const core::behavior::CallContext& ctx) const
    {
        return m_fulfill.fulfill(ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutListResponse>
    list(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_list.list(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutListResponse>
    list_profile(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_list_profile.list_profile(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutListResponse>
    list_by_filter(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_list_by_filter.list_by_filter(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutListResponse>
    list_by_filter_profile(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_list_by_filter_profile.list_by_filter_profile(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutListFilters>
    list_available_filters_for_merchant(const core::behavior::CallContext& ctx) const
    {
        return m_list_available_filters_for_merchant.list_available_filters_for_merchant(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutListFilters>
    list_available_filters_for_profile(const core::behavior::CallContext& ctx) const
    {
        return m_list_available_filters_for_profile.list_available_filters_for_profile(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutListFiltersV2>
    get_payout_filters(const core::behavior::CallContext& ctx) const
    {
        return m_get_payout_filters.get_payout_filters(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutListFiltersV2>
    get_payout_filters_profile(const core::behavior::CallContext& ctx) const
    {
        return m_get_payout_filters_profile.get_payout_filters_profile(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutsAggregateResponse>
    get_payouts_aggregates(const core::behavior::CallContext& ctx) const
    {
        return m_get_payouts_aggregates.get_payouts_aggregates(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutsAggregateResponse>
    get_payouts_aggregates_profile(const core::behavior::CallContext& ctx) const
    {
        return m_get_payouts_aggregates_profile.get_payouts_aggregates_profile(ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutsManualUpdateResponse>
    manual_update(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_manual_update.manual_update(dto, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    PayoutsCreateRuntime m_create;
    PayoutsRetrieveRuntime m_retrieve;
    PayoutsUpdateRuntime m_update;
    PayoutsConfirmRuntime m_confirm;
    PayoutsCancelRuntime m_cancel;
    PayoutsFulfillRuntime m_fulfill;
    PayoutsListRuntime m_list;
    PayoutsListProfileRuntime m_list_profile;
    PayoutsListByFilterRuntime m_list_by_filter;
    PayoutsListByFilterProfileRuntime m_list_by_filter_profile;
    PayoutsListAvailableFiltersForMerchantRuntime m_list_available_filters_for_merchant;
    PayoutsListAvailableFiltersForProfileRuntime m_list_available_filters_for_profile;
    PayoutsGetFiltersRuntime m_get_payout_filters;
    PayoutsGetFiltersProfileRuntime m_get_payout_filters_profile;
    PayoutsGetPayoutsAggregatesRuntime m_get_payouts_aggregates;
    PayoutsGetPayoutsAggregatesProfileRuntime m_get_payouts_aggregates_profile;
    PayoutsManualUpdateRuntime m_manual_update;
};

} // namespace app::handlers::payouts
