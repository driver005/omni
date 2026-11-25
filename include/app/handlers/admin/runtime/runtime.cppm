export module app_handlers_admin:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_organization_create;
import :runtime_organization_update;
import :runtime_organization_retrieve;
import :runtime_convert_organization_to_platform;
import :runtime_merchant_account_create;
import :runtime_retrieve_merchant_account;
import :runtime_merchant_account_list;
import :runtime_update_merchant_account;
import :runtime_delete_merchant_account;
import :runtime_connector_create;
import :runtime_connector_retrieve;
import :runtime_connector_list;
import :runtime_connector_list_profile;
import :runtime_connector_update;
import :runtime_connector_delete;
import :runtime_merchant_account_toggle_kv;
import :runtime_merchant_account_toggle_all_kv;
import :runtime_merchant_account_kv_status;
import :runtime_merchant_account_transfer_keys;
import :runtime_connector_webhook_register;
import :runtime_retrieve_connector_webhook;

export namespace app::handlers::admin {

class AdminRuntime
{
public:
    explicit AdminRuntime(app::context::AppContext& context) noexcept :
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

    [[nodiscard]] std::expected<OrganizationCreateResult, core::error::AppError>
    create_organization(const OrganizationCreateInput& input) const
    {
        return m_organization_create.create(input);
    }

    [[nodiscard]] std::expected<OrganizationUpdateResult, core::error::AppError>
    update_organization(const OrganizationUpdateInput& input) const
    {
        return m_organization_update.update(input);
    }

    [[nodiscard]] std::expected<OrganizationRetrieveResult, core::error::AppError>
    retrieve_organization(const OrganizationRetrieveInput& input) const
    {
        return m_organization_retrieve.retrieve(input);
    }

    [[nodiscard]] std::expected<ConvertOrganizationToPlatformResult, core::error::AppError>
    convert_organization_to_platform(const ConvertOrganizationToPlatformInput& input) const
    {
        return m_convert_organization_to_platform.convert(input);
    }

    [[nodiscard]] std::expected<MerchantAccountCreateResult, core::error::AppError>
    create_merchant_account(const MerchantAccountCreateInput& input) const
    {
        return m_merchant_account_create.create(input);
    }

    [[nodiscard]] std::expected<RetrieveMerchantAccountResult, core::error::AppError>
    retrieve_merchant_account(const RetrieveMerchantAccountInput& input) const
    {
        return m_retrieve_merchant_account.retrieve(input);
    }

    [[nodiscard]] std::expected<MerchantAccountListResult, core::error::AppError>
    list_merchant_accounts(const MerchantAccountListInput& input) const
    {
        return m_merchant_account_list.list(input);
    }

    [[nodiscard]] std::expected<UpdateMerchantAccountResult, core::error::AppError>
    update_merchant_account(const UpdateMerchantAccountInput& input) const
    {
        return m_update_merchant_account.update(input);
    }

    [[nodiscard]] std::expected<DeleteMerchantAccountResult, core::error::AppError>
    delete_merchant_account(const DeleteMerchantAccountInput& input) const
    {
        return m_delete_merchant_account.remove(input);
    }

    [[nodiscard]] std::expected<ConnectorCreateResult, core::error::AppError>
    create_connector(const ConnectorCreateInput& input) const
    {
        return m_connector_create.create(input);
    }

    [[nodiscard]] std::expected<ConnectorRetrieveResult, core::error::AppError>
    retrieve_connector(const ConnectorRetrieveInput& input) const
    {
        return m_connector_retrieve.retrieve(input);
    }

    [[nodiscard]] std::expected<ConnectorListResult, core::error::AppError>
    list_connectors(const ConnectorListInput& input) const
    {
        return m_connector_list.list(input);
    }

    [[nodiscard]] std::expected<ConnectorListProfileResult, core::error::AppError>
    list_connectors_profile(const ConnectorListProfileInput& input) const
    {
        return m_connector_list_profile.list(input);
    }

    [[nodiscard]] std::expected<ConnectorUpdateResult, core::error::AppError>
    update_connector(const ConnectorUpdateInput& input) const
    {
        return m_connector_update.update(input);
    }

    [[nodiscard]] std::expected<ConnectorDeleteResult, core::error::AppError>
    delete_connector(const ConnectorDeleteInput& input) const
    {
        return m_connector_delete.remove(input);
    }

    [[nodiscard]] std::expected<MerchantAccountToggleKvResult, core::error::AppError>
    toggle_kv(const MerchantAccountToggleKvInput& input) const
    {
        return m_merchant_account_toggle_kv.toggle(input);
    }

    [[nodiscard]] std::expected<MerchantAccountToggleAllKvResult, core::error::AppError>
    toggle_all_kv(const MerchantAccountToggleAllKvInput& input) const
    {
        return m_merchant_account_toggle_all_kv.toggle_all(input);
    }

    [[nodiscard]] std::expected<MerchantAccountKvStatusResult, core::error::AppError>
    kv_status(const MerchantAccountKvStatusInput& input) const
    {
        return m_merchant_account_kv_status.status(input);
    }

    [[nodiscard]] std::expected<MerchantAccountTransferKeysResult, core::error::AppError>
    transfer_keys(const MerchantAccountTransferKeysInput& input) const
    {
        return m_merchant_account_transfer_keys.transfer(input);
    }

    [[nodiscard]] std::expected<ConnectorWebhookRegisterResult, core::error::AppError>
    register_webhook(const ConnectorWebhookRegisterInput& input) const
    {
        return m_connector_webhook_register.register_webhook(input);
    }

    [[nodiscard]] std::expected<RetrieveConnectorWebhookResult, core::error::AppError>
    fetch_webhook(const RetrieveConnectorWebhookInput& input) const
    {
        return m_retrieve_connector_webhook.fetch(input);
    }

private:
    app::context::AppContext& m_context;
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
