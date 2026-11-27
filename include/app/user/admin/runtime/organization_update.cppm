export module app_user:admin_runtime_organization_update;

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

import :admin_input;
import :admin_result;
import :admin_runtime_common;

export namespace app::handlers::admin {

class OrganizationUpdateRuntime
{
public:
    explicit OrganizationUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::
        expected<::app::model::organization::OrganizationResponse, core::error::AppError>
        update(const OrganizationUpdateInput& input) const
    {
        auto found = m_context.repository().find_organization_by_string(input.organization_id);
        if (!found.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "organization with the given id does not exist"
            )};
        }
        auto record = std::move(*found);
        if (input.request.get_organization_name().has_value()) {
            record.set_organization_name(*input.request.get_organization_name());
        }
        if (input.request.get_organization_details().has_value()) {
            record.set_organization_details(input.request.get_organization_details()->get_value());
        }
        if (input.request.get_metadata().has_value()) {
            record.set_metadata(input.request.get_metadata()->get_value());
        }
        if (input.request.get_platform_merchant_id().has_value()) {
            record.set_platform_merchant_id(input.request.get_platform_merchant_id()->get_value());
        }
        if (const auto duplicate =
                m_support.acquire_operation_lock("organization_update", input.organization_id)) {
            return std::unexpected{*duplicate};
        }
        m_context.repository().update_organization(record);
        m_support.release_operation_lock("organization_update", input.organization_id);

        ::app::model::organization::OrganizationResponse response;
        ::app::model::organization::OrganizationIdValue id;
        id.set_value(record.get_organization_id());
        response.set_organization_id(std::move(id));
        response.set_organization_name(record.get_organization_name());
        if (!record.get_organization_details().empty()) {
            ::app::model::organization::SecretSerdeValueValue details;
            details.set_value(record.get_organization_details());
            response.set_organization_details(std::move(details));
        }
        if (!record.get_metadata().empty()) {
            ::app::model::organization::SecretSerdeValueValue metadata;
            metadata.set_value(record.get_metadata());
            response.set_metadata(std::move(metadata));
        }

        m_context.metrics().record_flow("organization_update");
        m_context.metrics().counter("admin.organization_update").increment();
        m_context.events().publish("organization.updated", serde::Ser::encode_json(response));
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
