export module app_user:admin_runtime_convert_organization_to_platform;

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
import app_locking;
import app_events;
import app_keystore;
import app_services;

import :admin_input;
import :admin_result;
import :admin_runtime_common;

export namespace app::handlers::admin {

class ConvertOrganizationToPlatformRuntime
{
public:
    explicit ConvertOrganizationToPlatformRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<
        ::app::model::organization::ConvertOrganizationToPlatformResponse>
    convert(
        const ::app::model::organization::ConvertOrganizationToPlatformRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        if (m_context.config_store().get_value("platform", "enabled").value_or("true") == "false") {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Platform feature is not enabled"
            )};
        }
        auto found =
            m_context.repository().find_organization_by_string(ctx_string(ctx, "organization_id"));
        if (!found.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "Organization with the given id does not exist"
            )};
        }
        if (found->get_organization_type() == "platform" ||
            !found->get_platform_merchant_id().empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Organization is already of type Platform"
            )};
        }
        if (const auto duplicate = m_support.acquire_operation_lock(
                "convert_organization_to_platform",
                ctx_string(ctx, "organization_id")
            )) {
            return std::unexpected{*duplicate};
        }
        auto record = std::move(*found);
        record.set_organization_type("platform");
        const std::string platform_merchant_id{"merchant_" + m_support.uuid_simple()};
        record.set_platform_merchant_id(platform_merchant_id);
        m_context.repository().update_organization(record);

        std::string merchant_name{request.get_platform_merchant_name().value_or("")};
        if (merchant_name.empty()) {
            merchant_name = record.get_organization_name() + " Platform";
        }
        ::app::store::merchant_accounts::MerchantAccountRecord platform_account;
        platform_account.set_merchant_id(platform_merchant_id);
        platform_account.set_merchant_name(merchant_name);
        platform_account.set_organization_id(record.get_organization_id());
        platform_account.set_parent_merchant_id(record.get_organization_id());
        platform_account.set_merchant_account_type("platform");
        m_context.repository().save_merchant_account(std::move(platform_account));
        m_context.events().publish(
            "merchant_account.created",
            std::format(
                "{{\"merchant_id\":\"{}\",\"organization_id\":\"{}\"}}",
                platform_merchant_id,
                record.get_organization_id()
            )
        );
        m_support.release_operation_lock(
            "convert_organization_to_platform",
            ctx_string(ctx, "organization_id")
        );

        ::app::model::organization::ConvertOrganizationToPlatformResponse response;
        ::app::model::organization::OrganizationIdValue id;
        id.set_value(record.get_organization_id());
        response.set_organization_id(std::move(id));
        ::app::model::organization::OrganizationTypeValue type;
        type.set_value(record.get_organization_type());
        response.set_organization_type(std::move(type));
        ::app::model::organization::MerchantIdValue platform_id;
        platform_id.set_value(record.get_platform_merchant_id());
        response.set_platform_merchant_id(std::move(platform_id));

        m_context.logger().record_flow("convert_organization_to_platform");
        m_context.logger().counter("admin.convert_organization_to_platform").increment();
        m_context.events().publish(
            "organization.converted_to_platform",
            serde::Ser::encode_json(response)
        );
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
