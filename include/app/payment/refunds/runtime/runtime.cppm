export module app_payment:refunds_runtime;

import std;
import core_serde;
import core_error;
import app_context;

import :refunds_input;
import :refunds_result;
import :refunds_runtime_common;
import :refunds_runtime_create;
import :refunds_runtime_retrieve;
import :refunds_runtime_retrieve_gateway;
import :refunds_runtime_retrieve_body;
import :refunds_runtime_update;
import :refunds_runtime_reverse;
import :refunds_runtime_metadata_update;
import :refunds_runtime_list;
import :refunds_runtime_list_platform;
import :refunds_runtime_filter_platform;
import :refunds_runtime_list_profile;
import :refunds_runtime_filter_list;
import :refunds_runtime_filters;
import :refunds_runtime_filters_profile;
import :refunds_runtime_aggregates;
import :refunds_runtime_aggregates_profile;
import :refunds_runtime_manual_update;

export namespace app::handlers::refunds {

class RefundsRuntime
{
public:
    explicit RefundsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_retrieve{context},
        m_retrieve_gateway{context},
        m_retrieve_body{context},
        m_update{context},
        m_reverse{context},
        m_metadata_update{context},
        m_list{context},
        m_list_platform{context},
        m_filter_platform{context},
        m_list_profile{context},
        m_filter_list{context},
        m_filters{context},
        m_filters_profile{context},
        m_aggregates{context},
        m_aggregates_profile{context},
        m_manual_update{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundResponse>
    create(const core::behavior::CallContext& ctx) const
    {
        return m_create.create(ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundResponse>
        retrieve(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_retrieve.retrieve(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundResponse>
        retrieve_gateway(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_retrieve_gateway.retrieve_gateway(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundResponse>
        retrieve_body(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_retrieve_body.retrieve_body(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundResponse>
        update(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_update.update(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundResponse>
        reverse(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_reverse.reverse(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundResponse>
        metadata_update(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_metadata_update.metadata_update(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundListResponse>
        list(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_list.list(dto, ctx);
    }

        template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::PlatformRefundListResponse>
    list_platform(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_list_platform.list_platform(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::PlatformRefundListFilters>
        filter_platform(const core::behavior::CallContext& ctx) const
    {
        return m_filter_platform.filter_platform(ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundListResponse>
        list_profile(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_list_profile.list_profile(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundListMetaData>
    filter_list(const core::behavior::CallContext& ctx) const
    {
        return m_filter_list.filter_list(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundListFilters>
    filters(const core::behavior::CallContext& ctx) const
    {
        return m_filters.filters(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundListFilters>
    filters_profile(const core::behavior::CallContext& ctx) const
    {
        return m_filters_profile.filters_profile(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundAggregateResponse>
        aggregates(const core::behavior::CallContext& ctx) const
    {
        return m_aggregates.aggregates(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundAggregateResponse>
        aggregates_profile(const core::behavior::CallContext& ctx) const
    {
        return m_aggregates_profile.aggregates_profile(ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<serde::Value>
        manual_update(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_manual_update.manual_update(dto, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    RefundCreateRuntime m_create;
    RefundRetrieveRuntime m_retrieve;
    RefundRetrieveGatewayRuntime m_retrieve_gateway;
    RefundRetrieveBodyRuntime m_retrieve_body;
    RefundUpdateRuntime m_update;
    RefundReverseRuntime m_reverse;
    RefundMetadataUpdateRuntime m_metadata_update;
    RefundListRuntime m_list;
    RefundListPlatformRuntime m_list_platform;
    RefundFilterPlatformRuntime m_filter_platform;
    RefundListProfileRuntime m_list_profile;
    RefundFilterListRuntime m_filter_list;
    RefundFiltersRuntime m_filters;
    RefundFiltersProfileRuntime m_filters_profile;
    RefundAggregatesRuntime m_aggregates;
    RefundAggregatesProfileRuntime m_aggregates_profile;
    RefundManualUpdateRuntime m_manual_update;
};

} // namespace app::handlers::refunds
