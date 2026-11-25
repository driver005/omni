export module app_handlers_hierarchical_resources:runtime_link;

import std;
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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::hierarchical_resources {

class LinkHierarchicalResourceRuntime
{
public:
    explicit LinkHierarchicalResourceRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief core:822-878 link_hierarchical_resource. */
    [[nodiscard]] std::expected<LinkHierarchicalResourceResult, core::error::AppError>
    link(const LinkHierarchicalResourceInput& input) const
    {
        m_context.metrics().record_flow("HierarchicalResourcesLink");
        m_context.metrics().counter("hierarchical_resources_link_total").increment();

        // core:832-839 — find_resource_scope_id + org authorization.
        const auto stored =
            m_context.repository().find_hierarchical_resource_by_string(input.resource_id);
        if (!stored.has_value()) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "resource not found")
            };
        }
        if (!m_support.authorize_scope_id_belongs_to_org(
                stored->get_scope_id(),
                input.organization_id
            )) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "resource belongs to a different organization"
            )};
        }

        // core:841-847 — fetch_requestor_account (V1, merchant scoped).
        const auto account = m_support.fetch_requestor_account(
            input.requestor_type,
            input.requestor_id,
            input.merchant_id
        );
        if (!account) {
            return std::unexpected{std::move(account.error())};
        }
        // core:859-873 — resource type + Active-only is_linkable is enforced by
        // is_linkable in upstream; link itself relies on the caller having an Active
        // resource. The local guard preserves that precondition explicitly.
        auto payload = m_support.decode_resource_data(stored->get_data());
        if (stored->get_status() != m_support.status_active()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "resource is not active"
            )};
        }
        if (!input.merchant_id.empty() && !payload.merchant_id.empty() &&
            payload.merchant_id != input.merchant_id) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "resource belongs to a different merchant"
            )};
        }

        // core:238-301 on_link — re-encrypt the private key under the requestor's
        // key store and mirror the account attachment (local config/event substitute).
        const auto requestor_key = m_support.ensure_requestor_key(input.requestor_id);
        const auto rewrapped =
            app::crypto::AesWrap{}.encrypt(payload.encrypted_data, requestor_key);
        const auto link_id = m_support.link_resource_id(input.requestor_id);
        const auto link_data =
            "{\"resource_id\":" + serde::Ser::encode_json(input.resource_id) +
            ",\"requestor_id\":" + serde::Ser::encode_json(input.requestor_id) +
            ",\"requestor_type\":" + serde::Ser::encode_json(input.requestor_type) +
            ",\"encrypted_data\":" + serde::Ser::encode_json(rewrapped) + "}";
        auto link_record = m_context.repository().find_hierarchical_resource_by_string(link_id);
        if (!link_record.has_value()) {
            app::store::hierarchical_resources::HierarchicalResourceRecord fresh{};
            fresh.set_resource_id(link_id);
            fresh.set_scope_id(std::string{input.resource_id});
            fresh.set_resource_type("link");
            fresh.set_data(link_data);
            fresh.set_status(m_support.status_active());
            fresh.set_created_at(m_support.created_at_value());
            const auto created =
                m_context.repository().create_hierarchical_resource(std::move(fresh));
            if (!created) {
                return std::unexpected{created.error()};
            }
        } else {
            link_record->set_scope_id(std::string{input.resource_id});
            link_record->set_data(link_data);
            if (!m_context.repository().update_hierarchical_resource(*link_record)) {
                return std::unexpected{m_support.make_error(
                    core::error::AppErrorKind::INTERNAL_ERROR,
                    "failed to persist link"
                )};
            }
        }
        m_context.config_store().put(
            "hierarchical_resources.account_links",
            input.requestor_id,
            input.resource_id,
            input.requestor_type
        );
        m_context.events().publish(
            "hierarchical_resources.linked",
            "{\"resource_id\":\"" + input.resource_id + "\",\"requestor_id\":\"" +
                input.requestor_id + "\"}"
        );
        app::dto::hierarchical_resources::LinkHierarchicalResourceResponse response;
        response.set_id(m_support.resource_id_value(std::string{input.resource_id}));
        return LinkHierarchicalResourceResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    HierarchicalResourcesRuntimeSupport m_support;
};

} // namespace app::handlers::hierarchical_resources
