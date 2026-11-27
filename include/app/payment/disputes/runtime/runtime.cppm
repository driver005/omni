export module app_payment:disputes_runtime;

import std;
import interfaces;
import core_router;
import core_serde;
import core_cache;
import core_database;
import core_error;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import app_metrics;
import app_store;
import app_connector;
import app_workflow;
import app_events;
import app_locking;
import app_services;

import :disputes_input;
import :disputes_result;
import :disputes_runtime_common;
import :disputes_runtime_retrieve_dispute;
import :disputes_runtime_fetch_disputes;
import :disputes_runtime_retrieve_disputes_list;
import :disputes_runtime_retrieve_disputes_list_profile;
import :disputes_runtime_retrieve_disputes_list_for_platform;
import :disputes_runtime_get_disputes_filters;
import :disputes_runtime_get_disputes_filters_profile;
import :disputes_runtime_get_platform_disputes_filters;
import :disputes_runtime_accept_dispute;
import :disputes_runtime_submit_dispute_evidence;
import :disputes_runtime_attach_dispute_evidence;
import :disputes_runtime_retrieve_dispute_evidence;
import :disputes_runtime_delete_dispute_evidence;
import :disputes_runtime_get_disputes_aggregate;
import :disputes_runtime_get_disputes_aggregate_profile;

export namespace app::handlers::disputes {

class DisputesRuntime
{
public:
    explicit DisputesRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_retrieve_dispute{context},
        m_fetch_disputes{context},
        m_retrieve_disputes_list{context},
        m_retrieve_disputes_list_profile{context},
        m_retrieve_disputes_list_for_platform{context},
        m_get_disputes_filters{context},
        m_get_disputes_filters_profile{context},
        m_get_platform_disputes_filters{context},
        m_accept_dispute{context},
        m_submit_dispute_evidence{context},
        m_attach_dispute_evidence{context},
        m_retrieve_dispute_evidence{context},
        m_delete_dispute_evidence{context},
        m_get_disputes_aggregate{context},
        m_get_disputes_aggregate_profile{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::disputes::DisputeResponse>
        retrieve_dispute(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_retrieve_dispute.retrieve_dispute(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<std::vector<::app::model::disputes::DisputeResponse>>
        fetch_disputes(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_fetch_disputes.fetch_disputes(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<std::vector<::app::model::disputes::DisputeResponse>>
        retrieve_disputes_list(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_retrieve_disputes_list.retrieve_disputes_list(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<std::vector<::app::model::disputes::DisputeResponse>>
        retrieve_disputes_list_profile(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_retrieve_disputes_list_profile.retrieve_disputes_list_profile(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::disputes::PlatformDisputeListResponse>
        retrieve_disputes_list_for_platform(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_retrieve_disputes_list_for_platform.retrieve_disputes_list_for_platform(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::disputes::DisputeListFilters>
    get_disputes_filters(const core::behavior::CallContext& ctx) const
    {
        return m_get_disputes_filters.get_disputes_filters(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::disputes::DisputeListFilters>
    get_disputes_filters_profile(const core::behavior::CallContext& ctx) const
    {
        return m_get_disputes_filters_profile.get_disputes_filters_profile(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::disputes::PlatformDisputeListFilters>
    get_platform_disputes_filters(const core::behavior::CallContext& ctx) const
    {
        return m_get_platform_disputes_filters.get_platform_disputes_filters(ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::disputes::DisputeResponse>
        accept_dispute(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_accept_dispute.accept_dispute(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::disputes::DisputeResponse>
        submit_dispute_evidence(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_submit_dispute_evidence.submit_dispute_evidence(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::dto::files::CreateFileResponse>
        attach_dispute_evidence(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_attach_dispute_evidence.attach_dispute_evidence(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<std::vector<::app::model::disputes::DisputeEvidenceBlock>>
        retrieve_dispute_evidence(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_retrieve_dispute_evidence.retrieve_dispute_evidence(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<serde::Value>
        delete_dispute_evidence(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_delete_dispute_evidence.delete_dispute_evidence(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::disputes::DisputesAggregateResponse>
    get_disputes_aggregate(const core::behavior::CallContext& ctx) const
    {
        return m_get_disputes_aggregate.get_disputes_aggregate(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::disputes::DisputesAggregateResponse>
    get_disputes_aggregate_profile(const core::behavior::CallContext& ctx) const
    {
        return m_get_disputes_aggregate_profile.get_disputes_aggregate_profile(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    DisputesRetrieveDisputeRuntime m_retrieve_dispute;
    DisputesFetchDisputesRuntime m_fetch_disputes;
    DisputesRetrieveDisputesListRuntime m_retrieve_disputes_list;
    DisputesRetrieveDisputesListProfileRuntime m_retrieve_disputes_list_profile;
    DisputesRetrieveDisputesListForPlatformRuntime m_retrieve_disputes_list_for_platform;
    DisputesGetDisputesFiltersRuntime m_get_disputes_filters;
    DisputesGetDisputesFiltersProfileRuntime m_get_disputes_filters_profile;
    DisputesGetPlatformDisputesFiltersRuntime m_get_platform_disputes_filters;
    DisputesAcceptDisputeRuntime m_accept_dispute;
    DisputesSubmitDisputeEvidenceRuntime m_submit_dispute_evidence;
    DisputesAttachDisputeEvidenceRuntime m_attach_dispute_evidence;
    DisputesRetrieveDisputeEvidenceRuntime m_retrieve_dispute_evidence;
    DisputesDeleteDisputeEvidenceRuntime m_delete_dispute_evidence;
    DisputesGetDisputesAggregateRuntime m_get_disputes_aggregate;
    DisputesGetDisputesAggregateProfileRuntime m_get_disputes_aggregate_profile;
};

} // namespace app::handlers::disputes
