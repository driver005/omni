export module app_payment:refunds_runtime;
import core_behavior;

import std;
import core_serde;
import core_error;
import app_context;
import app_context_core;
import app_model;

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

    [[nodiscard]] core::behavior::Outcome<::app::model::Refund>
    create(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        return m_create.create(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::Refund>
    retrieve(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        return m_retrieve.retrieve(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::Refund>
    retrieve_gateway(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        return m_retrieve_gateway.retrieve_gateway(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::Refund>
    retrieve_body(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        return m_retrieve_body.retrieve_body(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::Refund>
    update(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        return m_update.update(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::Refund>
    reverse(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        return m_reverse.reverse(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::Refund>
    metadata_update(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        return m_metadata_update.metadata_update(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        return m_list.list(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_platform(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        return m_list_platform.list_platform(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    filter_platform(const core::behavior::CallContext& ctx) const
    {
        return m_filter_platform.filter_platform(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_profile(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        return m_list_profile.list_profile(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    filter_list(const core::behavior::CallContext& ctx) const
    {
        return m_filter_list.filter_list(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    filters(const core::behavior::CallContext& ctx) const
    {
        return m_filters.filters(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    filters_profile(const core::behavior::CallContext& ctx) const
    {
        return m_filters_profile.filters_profile(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    aggregates(const core::behavior::CallContext& ctx) const
    {
        return m_aggregates.aggregates(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    aggregates_profile(const core::behavior::CallContext& ctx) const
    {
        return m_aggregates_profile.aggregates_profile(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    manual_update(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
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
