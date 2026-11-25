export module app_handlers_payouts:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_create;
import :runtime_retrieve;
import :runtime_update;
import :runtime_confirm;
import :runtime_cancel;
import :runtime_fulfill;
import :runtime_list;
import :runtime_list_profile;
import :runtime_list_by_filter;
import :runtime_list_by_filter_profile;
import :runtime_list_available_filters_for_merchant;
import :runtime_list_available_filters_for_profile;
import :runtime_get_payout_filters;
import :runtime_get_payout_filters_profile;
import :runtime_get_payouts_aggregates;
import :runtime_get_payouts_aggregates_profile;
import :runtime_manual_update;

export namespace app::handlers::payouts {

class PayoutsRuntime
{
public:
    explicit PayoutsRuntime(app::context::AppContext& context) noexcept :
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

    [[nodiscard]] std::expected<PayoutsCreateResult, core::error::AppError>
    create(const PayoutsCreateInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<PayoutsRetrieveResult, core::error::AppError>
    retrieve(const PayoutsRetrieveInput& input) const
    {
        return m_retrieve.retrieve(input);
    }

    [[nodiscard]] std::expected<PayoutsUpdateResult, core::error::AppError>
    update(const PayoutsUpdateInput& input) const
    {
        return m_update.update(input);
    }

    [[nodiscard]] std::expected<PayoutsConfirmResult, core::error::AppError>
    confirm(const PayoutsConfirmInput& input) const
    {
        return m_confirm.confirm(input);
    }

    [[nodiscard]] std::expected<PayoutsCancelResult, core::error::AppError>
    cancel(const PayoutsCancelInput& input) const
    {
        return m_cancel.cancel(input);
    }

    [[nodiscard]] std::expected<PayoutsFulfillResult, core::error::AppError>
    fulfill(const PayoutsFulfillInput& input) const
    {
        return m_fulfill.fulfill(input);
    }

    [[nodiscard]] std::expected<PayoutsListResult, core::error::AppError>
    list(const PayoutsListInput& input) const
    {
        return m_list.list(input);
    }

    [[nodiscard]] std::expected<PayoutsListProfileResult, core::error::AppError>
    list_profile(const PayoutsListProfileInput& input) const
    {
        return m_list_profile.list_profile(input);
    }

    [[nodiscard]] std::expected<PayoutsListByFilterResult, core::error::AppError>
    list_by_filter(const PayoutsListByFilterInput& input) const
    {
        return m_list_by_filter.list_by_filter(input);
    }

    [[nodiscard]] std::expected<PayoutsListByFilterProfileResult, core::error::AppError>
    list_by_filter_profile(const PayoutsListByFilterProfileInput& input) const
    {
        return m_list_by_filter_profile.list_by_filter_profile(input);
    }

    [[nodiscard]] std::expected<PayoutsListAvailableFiltersForMerchantResult, core::error::AppError>
    list_available_filters_for_merchant(
        const PayoutsListAvailableFiltersForMerchantInput& input
    ) const
    {
        return m_list_available_filters_for_merchant.list_available_filters_for_merchant(input);
    }

    [[nodiscard]] std::expected<PayoutsListAvailableFiltersForProfileResult, core::error::AppError>
    list_available_filters_for_profile(
        const PayoutsListAvailableFiltersForProfileInput& input
    ) const
    {
        return m_list_available_filters_for_profile.list_available_filters_for_profile(input);
    }

    [[nodiscard]] std::expected<PayoutsGetFiltersResult, core::error::AppError>
    get_payout_filters(const PayoutsGetFiltersInput& input) const
    {
        return m_get_payout_filters.get_payout_filters(input);
    }

    [[nodiscard]] std::expected<PayoutsGetFiltersProfileResult, core::error::AppError>
    get_payout_filters_profile(const PayoutsGetFiltersProfileInput& input) const
    {
        return m_get_payout_filters_profile.get_payout_filters_profile(input);
    }

    [[nodiscard]] std::expected<PayoutsAggregatesResult, core::error::AppError>
    get_payouts_aggregates(const PayoutsAggregatesInput& input) const
    {
        return m_get_payouts_aggregates.get_payouts_aggregates(input);
    }

    [[nodiscard]] std::expected<PayoutsAggregatesProfileResult, core::error::AppError>
    get_payouts_aggregates_profile(const PayoutsAggregatesProfileInput& input) const
    {
        return m_get_payouts_aggregates_profile.get_payouts_aggregates_profile(input);
    }

    [[nodiscard]] std::expected<PayoutsManualUpdateResult, core::error::AppError>
    manual_update(const PayoutsManualUpdateInput& input) const
    {
        return m_manual_update.manual_update(input);
    }

private:
    app::context::AppContext& m_context;
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
