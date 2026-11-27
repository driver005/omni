export module app_user:admin_runtime_connector_update;

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

class ConnectorUpdateRuntime
{
public:
    explicit ConnectorUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::MerchantConnectorResponse> update(
        const ::app::dto::admin::MerchantConnectorUpdate& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        auto found = m_context.repository().find_merchant_connector_by_string(
            ctx_string(ctx, "connector_id")
        );
        if (!found.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "Merchant connector account does not exist in our records"
            )};
        }
        if (const auto duplicate = m_support.acquire_operation_lock(
                "connector_update",
                ctx_string(ctx, "connector_id")
            )) {
            return std::unexpected{*duplicate};
        }
        auto record = std::move(*found);
        if (request.get_connector_label().has_value()) {
            record.set_connector_label(*request.get_connector_label());
        }
        if (request.get_metadata().has_value()) {
            record.set_metadata(request.get_metadata()->get_value());
        }
        if (request.get_disabled().has_value()) {
            record.set_disabled(*request.get_disabled());
        }
        if (request.get_status().has_value()) {
            record.set_status(request.get_status()->get_value());
        }
        if (request.get_connector_account_details().has_value()) {
            record.set_connector_account_details(
                ::app::crypto::AesWrap{}.encrypt(
                    request.get_connector_account_details()->get_value(),
                    m_support.connector_encryption_key(
                        record.get_merchant_id(),
                        ctx_string(ctx, "connector_id")
                    )
                )
            );
        }
        m_context.repository().update_merchant_connector(record);
        m_support.release_operation_lock("connector_update", ctx_string(ctx, "connector_id"));

        ::app::dto::admin::MerchantConnectorResponse response;
        response.set_connector_type(m_support.connector_type_value(record.get_connector_type()));
        response.set_connector_name(m_support.connector_value(record.get_connector_name()));
        response.set_connector_label(record.get_connector_label());
        response.set_id(m_support.connector_account_id(record.get_merchant_connector_id()));
        response.set_profile_id(m_support.profile_id(record.get_profile_id()));
        response.set_connector_account_details(
            m_support.secret_value(record.get_connector_account_details())
        );
        response.set_disabled(record.get_disabled());
        if (!record.get_metadata().empty()) {
            response.set_metadata(m_support.secret_value(record.get_metadata()));
        }
        if (!record.get_status().empty()) {
            response.set_status(m_support.connector_status_value(record.get_status()));
        }

        m_context.logger().record_flow("connector_update");
        m_context.logger().counter("admin.connector_update").increment();
        m_context.events().publish("merchant_connector.updated", serde::Ser::encode_json(response));
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
