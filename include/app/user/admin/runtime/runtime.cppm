export module app_user:admin_runtime;

import std;
import core_behavior;
import core_error;
import app_context;

import :admin_input;
import :admin_result;
import :admin_runtime_common;
import :admin_runtime_organization_create;
import :admin_runtime_organization_update;
import :admin_runtime_organization_retrieve;
import :admin_runtime_convert_organization_to_platform;
import :admin_runtime_merchant_account_create;
import :admin_runtime_retrieve_merchant_account;
import :admin_runtime_merchant_account_list;
import :admin_runtime_update_merchant_account;
import :admin_runtime_delete_merchant_account;
import :admin_runtime_connector_create;
import :admin_runtime_connector_retrieve;
import :admin_runtime_connector_list;
import :admin_runtime_connector_list_profile;
import :admin_runtime_connector_update;
import :admin_runtime_connector_delete;
import :admin_runtime_merchant_account_toggle_kv;
import :admin_runtime_merchant_account_toggle_all_kv;
import :admin_runtime_merchant_account_kv_status;
import :admin_runtime_merchant_account_transfer_keys;
import :admin_runtime_connector_webhook_register;
import :admin_runtime_retrieve_connector_webhook;

export namespace app::handlers::admin {

class AdminRuntime
{
public:
    explicit AdminRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_organization_create{context},
        m_organization_update{context},
        m_organization_retrieve{context},
        m_convert_organization_to_platform{context},
        m_merchant_account_create{context},
        m_retrieve_merchant_account{context},
        m_merchant_account_list{context},
        m_update_merchant_account{context},
        m_delete_merchant_account{context},
        m_connector_create{context},
        m_connector_retrieve{context},
        m_connector_list{context},
        m_connector_list_profile{context},
        m_connector_update{context},
        m_connector_delete{context},
        m_merchant_account_toggle_kv{context},
        m_merchant_account_toggle_all_kv{context},
        m_merchant_account_kv_status{context},
        m_merchant_account_transfer_keys{context},
        m_connector_webhook_register{context},
        m_retrieve_connector_webhook{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::organization::OrganizationResponse>
        create_organization(const ::app::model::organization::OrganizationCreateRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_organization_create.create(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::organization::OrganizationResponse>
        update_organization(const ::app::model::organization::OrganizationUpdateRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_organization_update.update(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::organization::OrganizationResponse>
        retrieve_organization(const ::core::behavior::CallContext& ctx) const
    {
        return m_organization_retrieve.retrieve(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<
        ::app::model::organization::ConvertOrganizationToPlatformResponse>
    convert_organization_to_platform(const ::app::model::organization::ConvertOrganizationToPlatformRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_convert_organization_to_platform.convert(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::MerchantAccountResponse>
    create_merchant_account(const ::app::dto::admin::MerchantAccountCreate& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_merchant_account_create.create(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::MerchantAccountResponse>
    retrieve_merchant_account(const ::core::behavior::CallContext& ctx) const
    {
        return m_retrieve_merchant_account.retrieve(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
        list_merchant_accounts(const ::core::behavior::CallContext& ctx) const
    {
        return m_merchant_account_list.list(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::MerchantAccountResponse>
    update_merchant_account(const ::app::dto::admin::MerchantAccountUpdate& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_update_merchant_account.update(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::MerchantAccountDeleteResponse>
        delete_merchant_account(const ::core::behavior::CallContext& ctx) const
    {
        return m_delete_merchant_account.remove(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::MerchantConnectorResponse>
    create_connector(const ::app::dto::admin::MerchantConnectorCreate& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_connector_create.create(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::MerchantConnectorResponse>
    retrieve_connector(const ::core::behavior::CallContext& ctx) const
    {
        return m_connector_retrieve.retrieve(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<
        std::vector<::app::dto::admin::MerchantConnectorListResponse>>
    list_connectors(const ::core::behavior::CallContext& ctx) const
    {
        return m_connector_list.list(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<
        std::vector<::app::dto::admin::MerchantConnectorListResponse>>
    list_connectors_profile(const ::core::behavior::CallContext& ctx) const
    {
        return m_connector_list_profile.list(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::MerchantConnectorResponse>
    update_connector(const ::app::dto::admin::MerchantConnectorUpdate& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_connector_update.update(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::MerchantConnectorDeleteResponse>
        delete_connector(const ::core::behavior::CallContext& ctx) const
    {
        return m_connector_delete.remove(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::ToggleKVResponse>
    toggle_kv(const ::core::behavior::CallContext& ctx) const
    {
        return m_merchant_account_toggle_kv.toggle(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::ToggleAllKVResponse>
    toggle_all_kv(const ::core::behavior::CallContext& ctx) const
    {
        return m_merchant_account_toggle_all_kv.toggle_all(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::ToggleKVResponse>
    kv_status(const ::core::behavior::CallContext& ctx) const
    {
        return m_merchant_account_kv_status.status(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::TransferKeyResponse>
    transfer_keys(const ::core::behavior::CallContext& ctx) const
    {
        return m_merchant_account_transfer_keys.transfer(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::WebhookDetailsResponse>
    register_webhook(const ::app::dto::admin::WebhookDetailsRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_connector_webhook_register.register_webhook(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::WebhookDetailsResponse>
    fetch_webhook(const ::core::behavior::CallContext& ctx) const
    {
        return m_retrieve_connector_webhook.fetch(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    OrganizationCreateRuntime m_organization_create;
    OrganizationUpdateRuntime m_organization_update;
    OrganizationRetrieveRuntime m_organization_retrieve;
    ConvertOrganizationToPlatformRuntime m_convert_organization_to_platform;
    MerchantAccountCreateRuntime m_merchant_account_create;
    RetrieveMerchantAccountRuntime m_retrieve_merchant_account;
    MerchantAccountListRuntime m_merchant_account_list;
    UpdateMerchantAccountRuntime m_update_merchant_account;
    DeleteMerchantAccountRuntime m_delete_merchant_account;
    ConnectorCreateRuntime m_connector_create;
    ConnectorRetrieveRuntime m_connector_retrieve;
    ConnectorListRuntime m_connector_list;
    ConnectorListProfileRuntime m_connector_list_profile;
    ConnectorUpdateRuntime m_connector_update;
    ConnectorDeleteRuntime m_connector_delete;
    MerchantAccountToggleKvRuntime m_merchant_account_toggle_kv;
    MerchantAccountToggleAllKvRuntime m_merchant_account_toggle_all_kv;
    MerchantAccountKvStatusRuntime m_merchant_account_kv_status;
    MerchantAccountTransferKeysRuntime m_merchant_account_transfer_keys;
    ConnectorWebhookRegisterRuntime m_connector_webhook_register;
    RetrieveConnectorWebhookRuntime m_retrieve_connector_webhook;
};

} // namespace app::handlers::admin
