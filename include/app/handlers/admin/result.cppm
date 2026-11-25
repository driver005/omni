export module app_handlers_admin:result;

import std;
import core_serde;
import app_dto;
import app_model;

export namespace app::handlers::admin {

// Typed results inherit the exact wire shape of the DTO they wrap. The
// Serializable specialization reuses the DTO's FieldDesc tuple, which applies to
// the derived object just as it does to the base object. List responses carry
// typed vectors; the handler serializes them as top-level JSON arrays.

class OrganizationCreateResult : public app::model::organization::OrganizationResponse
{
public:
    OrganizationCreateResult() = default;

    explicit OrganizationCreateResult(app::model::organization::OrganizationResponse value) noexcept
        :
        OrganizationResponse{std::move(value)}
    {
    }
};

class OrganizationUpdateResult : public app::model::organization::OrganizationResponse
{
public:
    OrganizationUpdateResult() = default;

    explicit OrganizationUpdateResult(app::model::organization::OrganizationResponse value) noexcept
        :
        OrganizationResponse{std::move(value)}
    {
    }
};

class OrganizationRetrieveResult : public app::model::organization::OrganizationResponse
{
public:
    OrganizationRetrieveResult() = default;

    explicit OrganizationRetrieveResult(
        app::model::organization::OrganizationResponse value
    ) noexcept :
        OrganizationResponse{std::move(value)}
    {
    }
};

class ConvertOrganizationToPlatformResult :
    public app::model::organization::ConvertOrganizationToPlatformResponse
{
public:
    ConvertOrganizationToPlatformResult() = default;

    explicit ConvertOrganizationToPlatformResult(
        app::model::organization::ConvertOrganizationToPlatformResponse value
    ) noexcept :
        ConvertOrganizationToPlatformResponse{std::move(value)}
    {
    }
};

class MerchantAccountCreateResult : public app::dto::admin::MerchantAccountResponse
{
public:
    MerchantAccountCreateResult() = default;

    explicit MerchantAccountCreateResult(app::dto::admin::MerchantAccountResponse value) noexcept :
        MerchantAccountResponse{std::move(value)}
    {
    }
};

class UpdateMerchantAccountResult : public app::dto::admin::MerchantAccountResponse
{
public:
    UpdateMerchantAccountResult() = default;

    explicit UpdateMerchantAccountResult(app::dto::admin::MerchantAccountResponse value) noexcept :
        MerchantAccountResponse{std::move(value)}
    {
    }
};

class RetrieveMerchantAccountResult : public app::dto::admin::MerchantAccountResponse
{
public:
    RetrieveMerchantAccountResult() = default;

    explicit RetrieveMerchantAccountResult(app::dto::admin::MerchantAccountResponse value) noexcept
        :
        MerchantAccountResponse{std::move(value)}
    {
    }
};

class DeleteMerchantAccountResult : public app::dto::admin::MerchantAccountDeleteResponse
{
public:
    DeleteMerchantAccountResult() = default;

    explicit DeleteMerchantAccountResult(
        app::dto::admin::MerchantAccountDeleteResponse value
    ) noexcept :
        MerchantAccountDeleteResponse{std::move(value)}
    {
    }
};

class ConnectorCreateResult : public app::dto::admin::MerchantConnectorResponse
{
public:
    ConnectorCreateResult() = default;

    explicit ConnectorCreateResult(app::dto::admin::MerchantConnectorResponse value) noexcept :
        MerchantConnectorResponse{std::move(value)}
    {
    }
};

class ConnectorRetrieveResult : public app::dto::admin::MerchantConnectorResponse
{
public:
    ConnectorRetrieveResult() = default;

    explicit ConnectorRetrieveResult(app::dto::admin::MerchantConnectorResponse value) noexcept :
        MerchantConnectorResponse{std::move(value)}
    {
    }
};

class ConnectorUpdateResult : public app::dto::admin::MerchantConnectorResponse
{
public:
    ConnectorUpdateResult() = default;

    explicit ConnectorUpdateResult(app::dto::admin::MerchantConnectorResponse value) noexcept :
        MerchantConnectorResponse{std::move(value)}
    {
    }
};

class ConnectorDeleteResult : public app::dto::admin::MerchantConnectorDeleteResponse
{
public:
    ConnectorDeleteResult() = default;

    explicit ConnectorDeleteResult(app::dto::admin::MerchantConnectorDeleteResponse value) noexcept
        :
        MerchantConnectorDeleteResponse{std::move(value)}
    {
    }
};

class MerchantAccountToggleKvResult : public app::dto::admin::ToggleKVResponse
{
public:
    MerchantAccountToggleKvResult() = default;

    explicit MerchantAccountToggleKvResult(app::dto::admin::ToggleKVResponse value) noexcept :
        ToggleKVResponse{std::move(value)}
    {
    }
};

class MerchantAccountKvStatusResult : public app::dto::admin::ToggleKVResponse
{
public:
    MerchantAccountKvStatusResult() = default;

    explicit MerchantAccountKvStatusResult(app::dto::admin::ToggleKVResponse value) noexcept :
        ToggleKVResponse{std::move(value)}
    {
    }
};

class MerchantAccountToggleAllKvResult : public app::dto::admin::ToggleAllKVResponse
{
public:
    MerchantAccountToggleAllKvResult() = default;

    explicit MerchantAccountToggleAllKvResult(app::dto::admin::ToggleAllKVResponse value) noexcept :
        ToggleAllKVResponse{std::move(value)}
    {
    }
};

class MerchantAccountTransferKeysResult : public app::dto::admin::TransferKeyResponse
{
public:
    MerchantAccountTransferKeysResult() = default;

    explicit MerchantAccountTransferKeysResult(app::dto::admin::TransferKeyResponse value) noexcept
        :
        TransferKeyResponse{std::move(value)}
    {
    }
};

class ConnectorWebhookRegisterResult : public app::dto::admin::WebhookDetailsResponse
{
public:
    ConnectorWebhookRegisterResult() = default;

    explicit ConnectorWebhookRegisterResult(app::dto::admin::WebhookDetailsResponse value) noexcept
        :
        WebhookDetailsResponse{std::move(value)}
    {
    }
};

class RetrieveConnectorWebhookResult : public app::dto::admin::WebhookDetailsResponse
{
public:
    RetrieveConnectorWebhookResult() = default;

    explicit RetrieveConnectorWebhookResult(app::dto::admin::WebhookDetailsResponse value) noexcept
        :
        WebhookDetailsResponse{std::move(value)}
    {
    }
};

// List results carry typed vectors of the response DTOs. The handler serializes
// get_items() so the wire stays a top-level JSON array.

class MerchantAccountListResult
{
public:
    MerchantAccountListResult() = default;

    explicit MerchantAccountListResult(
        std::vector<app::dto::admin::MerchantAccountResponse> items
    ) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<app::dto::admin::MerchantAccountResponse>&
    get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<app::dto::admin::MerchantAccountResponse> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<app::dto::admin::MerchantAccountResponse> m_items{};
};

class ConnectorListResult
{
public:
    ConnectorListResult() = default;

    explicit ConnectorListResult(
        std::vector<app::dto::admin::MerchantConnectorListResponse> items
    ) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<app::dto::admin::MerchantConnectorListResponse>&
    get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<app::dto::admin::MerchantConnectorListResponse> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<app::dto::admin::MerchantConnectorListResponse> m_items{};
};

class ConnectorListProfileResult
{
public:
    ConnectorListProfileResult() = default;

    explicit ConnectorListProfileResult(
        std::vector<app::dto::admin::MerchantConnectorListResponse> items
    ) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<app::dto::admin::MerchantConnectorListResponse>&
    get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<app::dto::admin::MerchantConnectorListResponse> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<app::dto::admin::MerchantConnectorListResponse> m_items{};
};

} // namespace app::handlers::admin

#define OMNI_ADMIN_RESULT_SERIALIZER(Name, Dto, Table)                                             \
    template<>                                                                                     \
    struct serde::Serializable<app::handlers::admin::Name>                                         \
    {                                                                                              \
        static constexpr auto fields()                                                             \
        {                                                                                          \
            return serde::Serializable<Dto>::fields();                                             \
        }                                                                                          \
        static constexpr std::string_view table_name()                                             \
        {                                                                                          \
            return Table;                                                                          \
        }                                                                                          \
    };

OMNI_ADMIN_RESULT_SERIALIZER(
    OrganizationCreateResult,
    app::model::organization::OrganizationResponse,
    "admin_organization_create_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    OrganizationUpdateResult,
    app::model::organization::OrganizationResponse,
    "admin_organization_update_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    OrganizationRetrieveResult,
    app::model::organization::OrganizationResponse,
    "admin_organization_retrieve_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    ConvertOrganizationToPlatformResult,
    app::model::organization::ConvertOrganizationToPlatformResponse,
    "admin_convert_organization_to_platform_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    MerchantAccountCreateResult,
    app::dto::admin::MerchantAccountResponse,
    "admin_merchant_account_create_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    UpdateMerchantAccountResult,
    app::dto::admin::MerchantAccountResponse,
    "admin_update_merchant_account_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    RetrieveMerchantAccountResult,
    app::dto::admin::MerchantAccountResponse,
    "admin_retrieve_merchant_account_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    DeleteMerchantAccountResult,
    app::dto::admin::MerchantAccountDeleteResponse,
    "admin_delete_merchant_account_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    ConnectorCreateResult,
    app::dto::admin::MerchantConnectorResponse,
    "admin_connector_create_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    ConnectorRetrieveResult,
    app::dto::admin::MerchantConnectorResponse,
    "admin_connector_retrieve_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    ConnectorUpdateResult,
    app::dto::admin::MerchantConnectorResponse,
    "admin_connector_update_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    ConnectorDeleteResult,
    app::dto::admin::MerchantConnectorDeleteResponse,
    "admin_connector_delete_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    MerchantAccountToggleKvResult,
    app::dto::admin::ToggleKVResponse,
    "admin_merchant_account_toggle_kv_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    MerchantAccountKvStatusResult,
    app::dto::admin::ToggleKVResponse,
    "admin_merchant_account_kv_status_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    MerchantAccountToggleAllKvResult,
    app::dto::admin::ToggleAllKVResponse,
    "admin_merchant_account_toggle_all_kv_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    MerchantAccountTransferKeysResult,
    app::dto::admin::TransferKeyResponse,
    "admin_merchant_account_transfer_keys_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    ConnectorWebhookRegisterResult,
    app::dto::admin::WebhookDetailsResponse,
    "admin_connector_webhook_register_result"
)
OMNI_ADMIN_RESULT_SERIALIZER(
    RetrieveConnectorWebhookResult,
    app::dto::admin::WebhookDetailsResponse,
    "admin_retrieve_connector_webhook_result"
)

#undef OMNI_ADMIN_RESULT_SERIALIZER
