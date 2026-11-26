export module app_payment:refunds_runtime;

import std;
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

    [[nodiscard]] std::expected<RefundsCreateResult, core::error::AppError>
    create(const RefundsCreateInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<RefundsRetrieveResult, core::error::AppError>
    retrieve(const RefundsRetrieveInput& input) const
    {
        return m_retrieve.retrieve(input);
    }

    [[nodiscard]] std::expected<RefundsRetrieveGatewayResult, core::error::AppError>
    retrieve_gateway(const RefundsRetrieveGatewayInput& input) const
    {
        return m_retrieve_gateway.retrieve_gateway(input);
    }

    [[nodiscard]] std::expected<RefundsRetrieveBodyResult, core::error::AppError>
    retrieve_body(const RefundsRetrieveBodyInput& input) const
    {
        return m_retrieve_body.retrieve_body(input);
    }

    [[nodiscard]] std::expected<RefundsUpdateResult, core::error::AppError>
    update(const RefundsUpdateInput& input) const
    {
        return m_update.update(input);
    }

    [[nodiscard]] std::expected<RefundsReverseResult, core::error::AppError>
    reverse(const RefundsReverseInput& input) const
    {
        return m_reverse.reverse(input);
    }

    [[nodiscard]] std::expected<RefundsMetadataUpdateResult, core::error::AppError>
    metadata_update(const RefundsMetadataUpdateInput& input) const
    {
        return m_metadata_update.metadata_update(input);
    }

    [[nodiscard]] std::expected<RefundsListResult, core::error::AppError>
    list(const RefundsListInput& input) const
    {
        return m_list.list(input);
    }

    [[nodiscard]] std::expected<RefundsListPlatformResult, core::error::AppError>
    list_platform(const RefundsListPlatformInput& input) const
    {
        return m_list_platform.list_platform(input);
    }

    [[nodiscard]] std::expected<RefundsFilterPlatformResult, core::error::AppError>
    filter_platform(const RefundsFilterPlatformInput& input) const
    {
        return m_filter_platform.filter_platform(input);
    }

    [[nodiscard]] std::expected<RefundsListProfileResult, core::error::AppError>
    list_profile(const RefundsListProfileInput& input) const
    {
        return m_list_profile.list_profile(input);
    }

    [[nodiscard]] std::expected<RefundsFilterListResult, core::error::AppError>
    filter_list(const RefundsFilterListInput& input) const
    {
        return m_filter_list.filter_list(input);
    }

    [[nodiscard]] std::expected<RefundsFiltersResult, core::error::AppError>
    filters(const RefundsFiltersInput& input) const
    {
        return m_filters.filters(input);
    }

    [[nodiscard]] std::expected<RefundsFiltersProfileResult, core::error::AppError>
    filters_profile(const RefundsFiltersProfileInput& input) const
    {
        return m_filters_profile.filters_profile(input);
    }

    [[nodiscard]] std::expected<RefundsAggregatesResult, core::error::AppError>
    aggregates(const RefundsAggregatesInput& input) const
    {
        return m_aggregates.aggregates(input);
    }

    [[nodiscard]] std::expected<RefundsAggregatesProfileResult, core::error::AppError>
    aggregates_profile(const RefundsAggregatesProfileInput& input) const
    {
        return m_aggregates_profile.aggregates_profile(input);
    }

    [[nodiscard]] std::expected<RefundsManualUpdateResult, core::error::AppError>
    manual_update(const RefundsManualUpdateInput& input) const
    {
        return m_manual_update.manual_update(input);
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
