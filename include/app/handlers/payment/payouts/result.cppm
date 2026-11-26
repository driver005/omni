export module app_payment:payouts_result;

import std;
import core_serde;
import app_model;

export namespace app::handlers::payouts {

// Each result inherits the wire fields of the exact DTO the old core built and
// encoded, so field names, order and wrapper types stay byte-identical. The
// Serializable specialization reuses the DTO's FieldDesc tuple; those descriptors
// apply to the derived object just as they do to the base object.

class PayoutsCreateResult : public ::app::model::payouts::PayoutCreateResponse
{
public:
    PayoutsCreateResult() = default;

    explicit PayoutsCreateResult(::app::model::payouts::PayoutCreateResponse value) noexcept :
        PayoutCreateResponse{std::move(value)}
    {
    }
};

class PayoutsRetrieveResult : public ::app::model::payouts::PayoutCreateResponse
{
public:
    PayoutsRetrieveResult() = default;

    explicit PayoutsRetrieveResult(::app::model::payouts::PayoutCreateResponse value) noexcept :
        PayoutCreateResponse{std::move(value)}
    {
    }
};

class PayoutsUpdateResult : public ::app::model::payouts::PayoutCreateResponse
{
public:
    PayoutsUpdateResult() = default;

    explicit PayoutsUpdateResult(::app::model::payouts::PayoutCreateResponse value) noexcept :
        PayoutCreateResponse{std::move(value)}
    {
    }
};

class PayoutsConfirmResult : public ::app::model::payouts::PayoutCreateResponse
{
public:
    PayoutsConfirmResult() = default;

    explicit PayoutsConfirmResult(::app::model::payouts::PayoutCreateResponse value) noexcept :
        PayoutCreateResponse{std::move(value)}
    {
    }
};

class PayoutsCancelResult : public ::app::model::payouts::PayoutCreateResponse
{
public:
    PayoutsCancelResult() = default;

    explicit PayoutsCancelResult(::app::model::payouts::PayoutCreateResponse value) noexcept :
        PayoutCreateResponse{std::move(value)}
    {
    }
};

class PayoutsFulfillResult : public ::app::model::payouts::PayoutCreateResponse
{
public:
    PayoutsFulfillResult() = default;

    explicit PayoutsFulfillResult(::app::model::payouts::PayoutCreateResponse value) noexcept :
        PayoutCreateResponse{std::move(value)}
    {
    }
};

class PayoutsListResult : public ::app::model::payouts::PayoutListResponse
{
public:
    PayoutsListResult() = default;

    explicit PayoutsListResult(::app::model::payouts::PayoutListResponse value) noexcept :
        PayoutListResponse{std::move(value)}
    {
    }
};

class PayoutsListProfileResult : public ::app::model::payouts::PayoutListResponse
{
public:
    PayoutsListProfileResult() = default;

    explicit PayoutsListProfileResult(::app::model::payouts::PayoutListResponse value) noexcept :
        PayoutListResponse{std::move(value)}
    {
    }
};

class PayoutsListByFilterResult : public ::app::model::payouts::PayoutListResponse
{
public:
    PayoutsListByFilterResult() = default;

    explicit PayoutsListByFilterResult(::app::model::payouts::PayoutListResponse value) noexcept :
        PayoutListResponse{std::move(value)}
    {
    }
};

class PayoutsListByFilterProfileResult : public ::app::model::payouts::PayoutListResponse
{
public:
    PayoutsListByFilterProfileResult() = default;

    explicit PayoutsListByFilterProfileResult(
        ::app::model::payouts::PayoutListResponse value
    ) noexcept :
        PayoutListResponse{std::move(value)}
    {
    }
};

class PayoutsListAvailableFiltersForMerchantResult : public ::app::model::payouts::PayoutListFilters
{
public:
    PayoutsListAvailableFiltersForMerchantResult() = default;

    explicit PayoutsListAvailableFiltersForMerchantResult(
        ::app::model::payouts::PayoutListFilters value
    ) noexcept :
        PayoutListFilters{std::move(value)}
    {
    }
};

class PayoutsListAvailableFiltersForProfileResult : public ::app::model::payouts::PayoutListFilters
{
public:
    PayoutsListAvailableFiltersForProfileResult() = default;

    explicit PayoutsListAvailableFiltersForProfileResult(
        ::app::model::payouts::PayoutListFilters value
    ) noexcept :
        PayoutListFilters{std::move(value)}
    {
    }
};

class PayoutsGetFiltersResult : public ::app::model::payouts::PayoutListFiltersV2
{
public:
    PayoutsGetFiltersResult() = default;

    explicit PayoutsGetFiltersResult(::app::model::payouts::PayoutListFiltersV2 value) noexcept :
        PayoutListFiltersV2{std::move(value)}
    {
    }
};

class PayoutsGetFiltersProfileResult : public ::app::model::payouts::PayoutListFiltersV2
{
public:
    PayoutsGetFiltersProfileResult() = default;

    explicit PayoutsGetFiltersProfileResult(
        ::app::model::payouts::PayoutListFiltersV2 value
    ) noexcept :
        PayoutListFiltersV2{std::move(value)}
    {
    }
};

class PayoutsAggregatesResult : public ::app::model::payouts::PayoutsAggregateResponse
{
public:
    PayoutsAggregatesResult() = default;

    explicit PayoutsAggregatesResult(::app::model::payouts::PayoutsAggregateResponse value) noexcept
        :
        PayoutsAggregateResponse{std::move(value)}
    {
    }
};

class PayoutsAggregatesProfileResult : public ::app::model::payouts::PayoutsAggregateResponse
{
public:
    PayoutsAggregatesProfileResult() = default;

    explicit PayoutsAggregatesProfileResult(
        ::app::model::payouts::PayoutsAggregateResponse value
    ) noexcept :
        PayoutsAggregateResponse{std::move(value)}
    {
    }
};

class PayoutsManualUpdateResult : public ::app::model::payouts::PayoutsManualUpdateResponse
{
public:
    PayoutsManualUpdateResult() = default;

    explicit PayoutsManualUpdateResult(
        ::app::model::payouts::PayoutsManualUpdateResponse value
    ) noexcept :
        PayoutsManualUpdateResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::payouts

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsCreateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutCreateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_create_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsRetrieveResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutCreateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_retrieve_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsUpdateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutCreateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_update_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsConfirmResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutCreateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_confirm_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsCancelResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutCreateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_cancel_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsFulfillResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutCreateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_fulfill_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsListResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutListResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_list_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsListProfileResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutListResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_list_profile_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsListByFilterResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutListResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_list_by_filter_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsListByFilterProfileResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutListResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_list_by_filter_profile_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsListAvailableFiltersForMerchantResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutListFilters>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_list_available_filters_for_merchant_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsListAvailableFiltersForProfileResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutListFilters>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_list_available_filters_for_profile_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsGetFiltersResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutListFiltersV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_get_filters_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsGetFiltersProfileResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutListFiltersV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_get_filters_profile_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsAggregatesResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutsAggregateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_aggregates_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsAggregatesProfileResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutsAggregateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_aggregates_profile_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::payouts::PayoutsManualUpdateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payouts::PayoutsManualUpdateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_manual_update_result";
    }
};
