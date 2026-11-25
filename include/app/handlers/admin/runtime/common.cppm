export module app_handlers_admin:runtime_common;

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

export namespace app::handlers::admin {

/**
 * @brief Shared business helpers for the admin sub-runtimes.
 *
 * Ports the original AdminRouteHandlers private helpers verbatim: operation
 * locks, connector validation/encryption keys, KV soft-kill guard, id and DTO
 * value wrappers. Holds the AppContext by reference.
 */
class AdminRuntimeSupport
{
public:
    explicit AdminRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::string
    operation_lock_key(std::string_view operation, std::string_view resource) const
    {
        return std::format("admin.{}.{}", operation, resource);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    acquire_operation_lock(std::string_view operation, std::string_view resource) const
    {
        const auto acquired = m_context.locks().acquire(
            operation_lock_key(operation, resource),
            std::int64_t{10},
            "admin-handler"
        );
        if (!acquired.has_value()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::DUPLICATE_REQUEST,
                "duplicate admin operation in progress"
            );
        }
        return std::nullopt;
    }

    void release_operation_lock(std::string_view operation, std::string_view resource) const
    {
        m_context.locks().release(operation_lock_key(operation, resource));
    }

    [[nodiscard]] bool is_vault_connector(std::string_view connector_name) const noexcept
    {
        return connector_name == "vgs" || connector_name == "hyperswitch_vault" ||
               connector_name == "tokenex";
    }

    [[nodiscard]] std::optional<core::error::AppError>
    validate_connector_create(std::string_view merchant_id, std::string_view connector_name) const
    {
        if (connector_name.empty()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing connector name"
            );
        }
        const auto account = m_context.repository().find_merchant_account_by_string(merchant_id);
        if (!account.has_value()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "merchant account not found"
            );
        }
        const bool dummy_enabled{
            m_context.config_store().get_value("dummy_connector", "enabled").value_or("false") ==
            "true"
        };
        if (connector_name == "dummy" && !dummy_enabled) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid connector name"
            );
        }
        const bool vault = is_vault_connector(connector_name);
        const std::string account_type{account->get_merchant_account_type()};
        if (account_type == "connected" && vault) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Vault connectors can only be configured by platform or standard merchants, not "
                "connected merchants"
            );
        }
        if (account_type == "platform" && !vault) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Platform merchants can only configure vault connectors"
            );
        }
        if (m_context.config_store().is_killed(connector_name)) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                std::format("connector killed: {}", connector_name)
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string
    connector_encryption_key(std::string_view merchant, std::string_view connector) const
    {
        const auto stored = m_context.key_store().get("connector_data_key", connector);
        return stored.has_value() ? stored->value() : connector_key(merchant, connector);
    }

    [[nodiscard]] std::optional<core::error::AppError> kv_soft_kill_guard() const
    {
        if (m_context.config_store().get_value("kv", "soft_kill").value_or("false") == "true") {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Kv cannot be enabled when application is in soft_kill_mode"
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string uuid_simple() const
    {
        std::string value{m_context.ids().uuid_v4()};
        std::erase(value, '-');
        return value;
    }

    [[nodiscard]] std::string
    connector_key(std::string_view merchant, std::string_view connector) const
    {
        return app::crypto::KeyedHasher{"admin.connector.key"}.hash(
            std::string{merchant} + ":" + std::string{connector}
        );
    }

    [[nodiscard]] app::dto::admin::MerchantIdValue merchant_id_value(std::string value) const
    {
        app::dto::admin::MerchantIdValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::OrganizationIdValue
    organization_id_value(std::string value) const
    {
        app::dto::admin::OrganizationIdValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::ConnectorValue connector_value(std::string value) const
    {
        app::dto::admin::ConnectorValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::ConnectorTypeValue connector_type_value(std::string value) const
    {
        app::dto::admin::ConnectorTypeValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::ConnectorStatusValue
    connector_status_value(std::string value) const
    {
        app::dto::admin::ConnectorStatusValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::SecretSerdeValueValue secret_value(std::string value) const
    {
        app::dto::admin::SecretSerdeValueValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::MerchantConnectorAccountIdValue
    connector_account_id(std::string value) const
    {
        app::dto::admin::MerchantConnectorAccountIdValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::ProfileIdValue profile_id(std::string value) const
    {
        app::dto::admin::ProfileIdValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::OptionalEncryptableNameValue
    encryptable_name(std::string value) const
    {
        app::dto::admin::OptionalEncryptableNameValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] std::vector<app::dto::admin::MerchantConnectorListResponse>
    list_payment_connectors(std::string_view merchant_id, std::string_view profile_id_filter) const
    {
        std::vector<app::dto::admin::MerchantConnectorListResponse> response;
        for (const auto& record: m_context.repository().list_merchant_connectors()) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) {
                continue;
            }
            if (!profile_id_filter.empty() && record.get_profile_id() != profile_id_filter) {
                continue;
            }
            app::dto::admin::MerchantConnectorListResponse item;
            item.set_connector_type(connector_type_value(record.get_connector_type()));
            item.set_connector_name(record.get_connector_name());
            item.set_connector_label(record.get_connector_label());
            item.set_merchant_connector_id(
                connector_account_id(record.get_merchant_connector_id())
            );
            item.set_profile_id(profile_id(record.get_profile_id()));
            item.set_disabled(record.get_disabled());
            item.set_status(connector_status_value(record.get_status()));
            response.push_back(std::move(item));
        }
        return response;
    }

    [[nodiscard]] std::vector<app::dto::admin::MerchantConnectorListResponse>
    list_connectors_for_a_profile(std::string_view profile_id_filter) const
    {
        std::vector<app::dto::admin::MerchantConnectorListResponse> response;
        for (const auto& record: m_context.repository().list_merchant_connectors()) {
            if (!profile_id_filter.empty() && record.get_profile_id() != profile_id_filter) {
                continue;
            }
            app::dto::admin::MerchantConnectorListResponse item;
            item.set_connector_type(connector_type_value(record.get_connector_type()));
            item.set_connector_name(record.get_connector_name());
            item.set_connector_label(record.get_connector_label());
            item.set_merchant_connector_id(
                connector_account_id(record.get_merchant_connector_id())
            );
            item.set_profile_id(this->profile_id(record.get_profile_id()));
            item.set_disabled(record.get_disabled());
            item.set_status(connector_status_value(record.get_status()));
            response.push_back(std::move(item));
        }
        return response;
    }

private:
    app::context::AppContext& m_context;
};

} // namespace app::handlers::admin
