export module app_handlers_refunds:result;

import std;
import core_serde;
import app_model;

export namespace app::handlers::refunds {

// Each DTO-shaped response inherits its wire fields from the exact DTO the old core
// built and encoded. Reusing the DTO's Serializable field tuple keeps field names,
// order and wrapper types byte-identical.

#define OMNI_REFUND_RESULT(Type, Base)                                                             \
    class Type : public app::model::refunds::Base                                                  \
    {                                                                                              \
    public:                                                                                        \
        Type() = default;                                                                          \
        explicit Type(app::model::refunds::Base value) noexcept :                                  \
            Base{std::move(value)}                                                                 \
        {                                                                                          \
        }                                                                                          \
    };

OMNI_REFUND_RESULT(RefundsCreateResult, RefundResponse)
OMNI_REFUND_RESULT(RefundsRetrieveResult, RefundResponse)
OMNI_REFUND_RESULT(RefundsRetrieveGatewayResult, RefundResponse)
OMNI_REFUND_RESULT(RefundsRetrieveBodyResult, RefundResponse)
OMNI_REFUND_RESULT(RefundsUpdateResult, RefundResponse)
OMNI_REFUND_RESULT(RefundsReverseResult, RefundResponse)
OMNI_REFUND_RESULT(RefundsMetadataUpdateResult, RefundResponse)
OMNI_REFUND_RESULT(RefundsListResult, RefundListResponse)
OMNI_REFUND_RESULT(RefundsListPlatformResult, PlatformRefundListResponse)
OMNI_REFUND_RESULT(RefundsFilterPlatformResult, PlatformRefundListFilters)
OMNI_REFUND_RESULT(RefundsListProfileResult, RefundListResponse)
OMNI_REFUND_RESULT(RefundsFilterListResult, RefundListMetaData)
OMNI_REFUND_RESULT(RefundsFiltersResult, RefundListFilters)
OMNI_REFUND_RESULT(RefundsFiltersProfileResult, RefundListFilters)
OMNI_REFUND_RESULT(RefundsAggregatesResult, RefundAggregateResponse)
OMNI_REFUND_RESULT(RefundsAggregatesProfileResult, RefundAggregateResponse)

#undef OMNI_REFUND_RESULT

// core/refunds.rs:2277 — ApplicationResponse::StatusOk (200, empty body). The typed
// carrier is an empty serde::Value document; the handler writes it with Ser::serialize.
class RefundsManualUpdateResult
{
public:
    [[nodiscard]] static serde::Value empty() noexcept
    {
        return serde::Value{serde::Value::Object{}};
    }

    RefundsManualUpdateResult() = default;

    explicit RefundsManualUpdateResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{empty()};
};

} // namespace app::handlers::refunds

#define OMNI_REFUND_RESULT_SERIALIZER(Type, Base, Table)                                           \
    template<>                                                                                     \
    struct serde::Serializable<app::handlers::refunds::Type>                                       \
    {                                                                                              \
        static constexpr auto fields()                                                             \
        {                                                                                          \
            return serde::Serializable<app::model::refunds::Base>::fields();                       \
        }                                                                                          \
        static constexpr std::string_view table_name()                                             \
        {                                                                                          \
            return Table;                                                                          \
        }                                                                                          \
    };

OMNI_REFUND_RESULT_SERIALIZER(RefundsCreateResult, RefundResponse, "refunds_create_result")
OMNI_REFUND_RESULT_SERIALIZER(RefundsRetrieveResult, RefundResponse, "refunds_retrieve_result")
OMNI_REFUND_RESULT_SERIALIZER(
    RefundsRetrieveGatewayResult,
    RefundResponse,
    "refunds_retrieve_gateway_result"
)
OMNI_REFUND_RESULT_SERIALIZER(
    RefundsRetrieveBodyResult,
    RefundResponse,
    "refunds_retrieve_body_result"
)
OMNI_REFUND_RESULT_SERIALIZER(RefundsUpdateResult, RefundResponse, "refunds_update_result")
OMNI_REFUND_RESULT_SERIALIZER(RefundsReverseResult, RefundResponse, "refunds_reverse_result")
OMNI_REFUND_RESULT_SERIALIZER(
    RefundsMetadataUpdateResult,
    RefundResponse,
    "refunds_metadata_update_result"
)
OMNI_REFUND_RESULT_SERIALIZER(RefundsListResult, RefundListResponse, "refunds_list_result")
OMNI_REFUND_RESULT_SERIALIZER(
    RefundsListPlatformResult,
    PlatformRefundListResponse,
    "refunds_list_platform_result"
)
OMNI_REFUND_RESULT_SERIALIZER(
    RefundsFilterPlatformResult,
    PlatformRefundListFilters,
    "refunds_filter_platform_result"
)
OMNI_REFUND_RESULT_SERIALIZER(
    RefundsListProfileResult,
    RefundListResponse,
    "refunds_list_profile_result"
)
OMNI_REFUND_RESULT_SERIALIZER(
    RefundsFilterListResult,
    RefundListMetaData,
    "refunds_filter_list_result"
)
OMNI_REFUND_RESULT_SERIALIZER(RefundsFiltersResult, RefundListFilters, "refunds_filters_result")
OMNI_REFUND_RESULT_SERIALIZER(
    RefundsFiltersProfileResult,
    RefundListFilters,
    "refunds_filters_profile_result"
)
OMNI_REFUND_RESULT_SERIALIZER(
    RefundsAggregatesResult,
    RefundAggregateResponse,
    "refunds_aggregates_result"
)
OMNI_REFUND_RESULT_SERIALIZER(
    RefundsAggregatesProfileResult,
    RefundAggregateResponse,
    "refunds_aggregates_profile_result"
)

#undef OMNI_REFUND_RESULT_SERIALIZER
