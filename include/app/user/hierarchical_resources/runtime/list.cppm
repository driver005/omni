export module app_user:hierarchical_resources_runtime_list;

import std;
import core_behavior;
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

import :hierarchical_resources_input;
import :hierarchical_resources_result;
import :hierarchical_resources_runtime_common;

export namespace app::handlers::hierarchical_resources {

class ListHierarchicalResourcesRuntime
{
public:
    explicit ListHierarchicalResourcesRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief core:728-820 list_hierarchical_resources. */
    [[nodiscard]] core::behavior::Outcome<
        ::app::dto::hierarchical_resources::ListHierarchicalResourcesResponse>
    list(const ::app::dto::hierarchical_resources::ListHierarchicalResourcesRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        m_context.metrics().record_flow("HierarchicalResourcesList");
        m_context.metrics().counter("hierarchical_resources_list_total").increment();

        const auto& request = request;

        // core:750-775 — effective id resolves only when BOTH scope_type and
        // scope_id are present; otherwise it is None.
        std::string effective_resource_id{};
        if (request.get_scope_type().has_value() && request.get_scope_id().has_value() &&
            !request.get_scope_id()->empty()) {
            const auto account = m_support.fetch_requestor_account(
                request.get_scope_type()->get_value(),
                *request.get_scope_id(),
                ctx_string(ctx,"merchant_id")
            );
            if (!account) {
                return std::unexpected{std::move(account.error())};
            }
            effective_resource_id = m_support.resolve_apple_pay_certificate_data(*account);
        }

        const auto resource_type = request.get_resource_type().get_value();
        std::vector<::app::dto::hierarchical_resources::HierarchicalResourceSummary> summaries{};
        for (const auto& record: m_context.repository().list_hierarchical_resources()) {
            if (record.get_resource_type() != resource_type) {
                continue;
            }
            if (!ctx_string(ctx,"organization_id").empty() && record.get_scope_id() != ctx_string(ctx,"organization_id")) {
                continue;
            }
            const auto payload = m_support.decode_resource_data(record.get_data());
            if (!m_support.hierarchical_resource_is_linkable(payload)) {
                continue;
            }
            if (!ctx_string(ctx,"merchant_id").empty() && !payload.merchant_id.empty() &&
                payload.merchant_id != ctx_string(ctx,"merchant_id")) {
                continue;
            }

            const auto certificate_data =
                m_support.parse_apple_pay_certificate_payload(payload.data);
            ::app::dto::hierarchical_resources::HierarchicalResourceSummary summary;
            summary.set_id(m_support.resource_id_value(record.get_resource_id()));
            summary.set_display_schema(serde::RawJson{m_support.display_schema()});
            summary.set_display_data(serde::RawJson{m_support.display_data(certificate_data)});
            summary.set_created_at(m_support.datetime_value(record.get_created_at()));
            summary.set_is_linked(
                !effective_resource_id.empty() && effective_resource_id == record.get_resource_id()
            );
            summaries.emplace_back(std::move(summary));
        }
        ::app::dto::hierarchical_resources::ListHierarchicalResourcesResponse response;
        response.set_resources(std::move(summaries));
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    HierarchicalResourcesRuntimeSupport m_support;
};

} // namespace app::handlers::hierarchical_resources
