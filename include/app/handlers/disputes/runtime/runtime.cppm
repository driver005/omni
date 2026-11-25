export module app_handlers_disputes:runtime;

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

import :input;
import :result;
import :runtime_common;
import :runtime_retrieve_dispute;
import :runtime_fetch_disputes;
import :runtime_retrieve_disputes_list;
import :runtime_retrieve_disputes_list_profile;
import :runtime_retrieve_disputes_list_for_platform;
import :runtime_get_disputes_filters;
import :runtime_get_disputes_filters_profile;
import :runtime_get_platform_disputes_filters;
import :runtime_accept_dispute;
import :runtime_submit_dispute_evidence;
import :runtime_attach_dispute_evidence;
import :runtime_retrieve_dispute_evidence;
import :runtime_delete_dispute_evidence;
import :runtime_get_disputes_aggregate;
import :runtime_get_disputes_aggregate_profile;

export namespace app::handlers::disputes {

class DisputesRuntime
{
public:
    explicit DisputesRuntime(app::context::AppContext& context) noexcept :
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

    [[nodiscard]] std::expected<RetrieveDisputeResult, Error>
    retrieve_dispute(const RetrieveDisputeInput& input) const
    {
        return m_retrieve_dispute.retrieve_dispute(input);
    }

    [[nodiscard]] std::expected<FetchDisputesResult, Error>
    fetch_disputes(const FetchDisputesInput& input) const
    {
        return m_fetch_disputes.fetch_disputes(input);
    }

    [[nodiscard]] std::expected<RetrieveDisputesListResult, Error>
    retrieve_disputes_list(const RetrieveDisputesListInput& input) const
    {
        return m_retrieve_disputes_list.retrieve_disputes_list(input);
    }

    [[nodiscard]] std::expected<RetrieveDisputesListProfileResult, Error>
    retrieve_disputes_list_profile(const RetrieveDisputesListProfileInput& input) const
    {
        return m_retrieve_disputes_list_profile.retrieve_disputes_list_profile(input);
    }

    [[nodiscard]] std::expected<RetrieveDisputesListForPlatformResult, Error>
    retrieve_disputes_list_for_platform(const RetrieveDisputesListForPlatformInput& input) const
    {
        return m_retrieve_disputes_list_for_platform.retrieve_disputes_list_for_platform(input);
    }

    [[nodiscard]] std::expected<GetDisputesFiltersResult, Error>
    get_disputes_filters(const GetDisputesFiltersInput& input) const
    {
        return m_get_disputes_filters.get_disputes_filters(input);
    }

    [[nodiscard]] std::expected<GetDisputesFiltersProfileResult, Error>
    get_disputes_filters_profile(const GetDisputesFiltersProfileInput& input) const
    {
        return m_get_disputes_filters_profile.get_disputes_filters_profile(input);
    }

    [[nodiscard]] std::expected<GetPlatformDisputesFiltersResult, Error>
    get_platform_disputes_filters(const GetPlatformDisputesFiltersInput& input) const
    {
        return m_get_platform_disputes_filters.get_platform_disputes_filters(input);
    }

    [[nodiscard]] std::expected<AcceptDisputeResult, Error>
    accept_dispute(const AcceptDisputeInput& input) const
    {
        return m_accept_dispute.accept_dispute(input);
    }

    [[nodiscard]] std::expected<SubmitDisputeEvidenceResult, Error>
    submit_dispute_evidence(const SubmitDisputeEvidenceInput& input) const
    {
        return m_submit_dispute_evidence.submit_dispute_evidence(input);
    }

    [[nodiscard]] std::expected<AttachDisputeEvidenceResult, Error>
    attach_dispute_evidence(const AttachDisputeEvidenceInput& input) const
    {
        return m_attach_dispute_evidence.attach_dispute_evidence(input);
    }

    [[nodiscard]] std::expected<RetrieveDisputeEvidenceResult, Error>
    retrieve_dispute_evidence(const RetrieveDisputeEvidenceInput& input) const
    {
        return m_retrieve_dispute_evidence.retrieve_dispute_evidence(input);
    }

    [[nodiscard]] std::expected<DeleteDisputeEvidenceResult, Error>
    delete_dispute_evidence(const DeleteDisputeEvidenceInput& input) const
    {
        return m_delete_dispute_evidence.delete_dispute_evidence(input);
    }

    [[nodiscard]] std::expected<GetDisputesAggregateResult, Error>
    get_disputes_aggregate(const GetDisputesAggregateInput& input) const
    {
        return m_get_disputes_aggregate.get_disputes_aggregate(input);
    }

    [[nodiscard]] std::expected<GetDisputesAggregateProfileResult, Error>
    get_disputes_aggregate_profile(const GetDisputesAggregateProfileInput& input) const
    {
        return m_get_disputes_aggregate_profile.get_disputes_aggregate_profile(input);
    }

private:
    app::context::AppContext& m_context;
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
