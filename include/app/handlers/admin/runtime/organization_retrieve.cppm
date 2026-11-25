export module app_handlers_admin:runtime_organization_retrieve;

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
import app_locking;
import app_events;
import app_keystore;
import app_services;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::admin {

class OrganizationRetrieveRuntime
{
public:
    explicit OrganizationRetrieveRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<OrganizationRetrieveResult, core::error::AppError>
    retrieve(const OrganizationRetrieveInput& input) const
    {
        const auto found =
            m_context.repository().find_organization_by_string(input.organization_id);
        if (!found.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "Organization with the given id does not exist"
            )};
        }
        app::model::organization::OrganizationResponse response;
        app::model::organization::OrganizationIdValue id;
        id.set_value(found->get_organization_id());
        response.set_organization_id(std::move(id));
        response.set_organization_name(found->get_organization_name());
        if (!found->get_organization_details().empty()) {
            app::model::organization::SecretSerdeValueValue details;
            details.set_value(found->get_organization_details());
            response.set_organization_details(std::move(details));
        }
        if (!found->get_metadata().empty()) {
            app::model::organization::SecretSerdeValueValue metadata;
            metadata.set_value(found->get_metadata());
            response.set_metadata(std::move(metadata));
        }
        if (!found->get_organization_type().empty()) {
            app::model::organization::OrganizationTypeValue type;
            type.set_value(found->get_organization_type());
            response.set_organization_type(std::move(type));
        }
        if (!found->get_created_at().empty()) {
            app::model::organization::PrimitiveDateTimeValue created;
            created.set_value(found->get_created_at());
            response.set_created_at(std::move(created));
        }

        m_context.metrics().record_flow("organization_retrieve");
        m_context.metrics().counter("admin.organization_retrieve").increment();
        return OrganizationRetrieveResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
