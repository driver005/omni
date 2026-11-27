export module app_user:admin_runtime_organization_create;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_behavior;
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

import :admin_input;
import :admin_result;
import :admin_runtime_common;

export namespace app::handlers::admin {

class OrganizationCreateRuntime
{
public:
    explicit OrganizationCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::organization::OrganizationResponse> create(
        const ::app::model::organization::OrganizationCreateRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        for (const auto& existing: m_context.repository().list_organizations()) {
            if (existing.get_organization_name() == request.get_organization_name()) {
                return std::unexpected{core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::ALREADY_EXISTS,
                    "Organization with the given organization_name already exists"
                )};
            }
        }
        if (const auto duplicate = m_support.acquire_operation_lock(
                "organization_create",
                request.get_organization_name()
            )) {
            return std::unexpected{*duplicate};
        }
        ::app::store::organizations::OrganizationRecord record;
        record.set_organization_id("org_" + m_support.uuid_simple());
        record.set_organization_name(request.get_organization_name());
        record.set_organization_details(
            request.get_organization_details() ? request.get_organization_details()->get_value()
                                               : std::string{}
        );
        record.set_metadata(
            request.get_metadata() ? request.get_metadata()->get_value() : std::string{}
        );
        record.set_organization_type("merchant");
        const auto saved = m_context.repository().save_organization(std::move(record));
        m_support.release_operation_lock("organization_create", request.get_organization_name());

        ::app::model::organization::OrganizationResponse response;
        ::app::model::organization::OrganizationIdValue id;
        id.set_value(saved.get_organization_id());
        response.set_organization_id(std::move(id));
        response.set_organization_name(saved.get_organization_name());
        if (!saved.get_organization_details().empty()) {
            ::app::model::organization::SecretSerdeValueValue details;
            details.set_value(saved.get_organization_details());
            response.set_organization_details(std::move(details));
        }
        if (!saved.get_metadata().empty()) {
            ::app::model::organization::SecretSerdeValueValue metadata;
            metadata.set_value(saved.get_metadata());
            response.set_metadata(std::move(metadata));
        }
        ::app::model::organization::OrganizationTypeValue type;
        type.set_value(saved.get_organization_type());
        response.set_organization_type(std::move(type));

        m_context.logger().record_flow("organization_create");
        m_context.logger().counter("admin.organization_create").increment();
        m_context.events().publish("organization.created", serde::Ser::encode_json(response));
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
