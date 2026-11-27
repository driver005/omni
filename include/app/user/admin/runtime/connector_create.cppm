export module app_user:admin_runtime_connector_create;

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

class ConnectorCreateRuntime
{
public:
    explicit ConnectorCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::MerchantConnectorResponse>
    create(const ::app::dto::admin::MerchantConnectorCreate& request,
        const ::core::behavior::CallContext& ctx) const
    {
        const std::string connector_name{request.get_connector_name().get_value()};
        if (const auto invalid =
                m_support.validate_connector_create(ctx_string(ctx,"merchant_id"), connector_name)) {
            return std::unexpected{*invalid};
        }
        const std::string connector_id{"mca_" + m_support.uuid_simple()};
        if (m_context.repository().find_merchant_connector_by_string(connector_id).has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::ALREADY_EXISTS,
                "The merchant connector account with the specified profile_id and connector_label "
                "already exists in our records"
            )};
        }
        if (const auto duplicate =
                m_support.acquire_operation_lock("connector_create", connector_id)) {
            return std::unexpected{*duplicate};
        }
        m_context.key_store().put(
            "connector_data_key",
            connector_id,
            "AES-256",
            ::app::crypto::SecretString{m_context.ids().key_id()}
        );

        const std::string connector_type{request.get_connector_type().get_value()};
        const std::string account_plaintext{
            request.get_connector_account_details()
                ? request.get_connector_account_details()->get_value()
                : std::string{}
        };
        const std::string account_ciphertext{::app::crypto::AesWrap{}.encrypt(
            account_plaintext,
            m_support.connector_encryption_key(ctx_string(ctx,"merchant_id"), connector_id)
        )};

        ::app::store::merchant_connectors::MerchantConnectorRecord record;
        record.set_merchant_connector_id(connector_id);
        record.set_merchant_id(std::string{ctx_string(ctx,"merchant_id")});
        record.set_connector_name(connector_name);
        record.set_connector_type(connector_type);
        record.set_connector_label(request.get_connector_label().value_or(""));
        record.set_profile_id(request.get_profile_id().get_value());
        record.set_connector_account_details(account_ciphertext);
        record.set_metadata(
            request.get_metadata() ? request.get_metadata()->get_value() : std::string{}
        );
        record.set_status(
            request.get_status() ? request.get_status()->get_value() : std::string{}
        );
        record.set_disabled(request.get_disabled().value_or(false));
        const auto saved = m_context.repository().save_merchant_connector(std::move(record));
        m_support.release_operation_lock("connector_create", connector_id);

        ::app::dto::admin::MerchantConnectorResponse response;
        response.set_connector_type(m_support.connector_type_value(saved.get_connector_type()));
        response.set_connector_name(m_support.connector_value(saved.get_connector_name()));
        response.set_connector_label(saved.get_connector_label());
        response.set_id(m_support.connector_account_id(saved.get_merchant_connector_id()));
        response.set_profile_id(m_support.profile_id(saved.get_profile_id()));
        response.set_connector_account_details(
            m_support.secret_value(saved.get_connector_account_details())
        );
        response.set_disabled(saved.get_disabled());
        if (!saved.get_metadata().empty()) {
            response.set_metadata(m_support.secret_value(saved.get_metadata()));
        }
        if (!saved.get_status().empty()) {
            response.set_status(m_support.connector_status_value(saved.get_status()));
        }
        if (request.get_connector_webhook_details().has_value()) {
            response.set_connector_webhook_details(*request.get_connector_webhook_details());
        }
        if (request.get_payment_methods_enabled().has_value()) {
            response.set_payment_methods_enabled(*request.get_payment_methods_enabled());
        }

        m_context.metrics().record_flow("connector_create");
        m_context.metrics().counter("admin.connector_create").increment();
        m_context.events().publish("merchant_connector.created", serde::Ser::encode_json(response));
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
