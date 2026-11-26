export module app_user:admin_runtime_connector_retrieve;

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

class ConnectorRetrieveRuntime
{
public:
    explicit ConnectorRetrieveRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ConnectorRetrieveResult, core::error::AppError>
    retrieve(const ConnectorRetrieveInput& input) const
    {
        const auto found =
            m_context.repository().find_merchant_connector_by_string(input.connector_id);
        if (!found.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "Merchant connector account does not exist in our records"
            )};
        }
        // core/admin.rs:3029-3038 (v2) rejects a connector owned by another merchant.
        if (!input.merchant_id.empty() && found->get_merchant_id() != input.merchant_id) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                std::format(
                    "Invalid merchant_id {} provided for merchant_connector_account",
                    input.merchant_id
                )
            )};
        }
        const std::string decrypted{::app::crypto::AesWrap{}.decrypt(
            found->get_connector_account_details(),
            m_support.connector_encryption_key(found->get_merchant_id(), input.connector_id)
        )};
        ::app::dto::admin::MerchantConnectorResponse response;
        response.set_connector_type(m_support.connector_type_value(found->get_connector_type()));
        response.set_connector_name(m_support.connector_value(found->get_connector_name()));
        response.set_connector_label(found->get_connector_label());
        response.set_id(m_support.connector_account_id(found->get_merchant_connector_id()));
        response.set_profile_id(m_support.profile_id(found->get_profile_id()));
        response.set_connector_account_details(m_support.secret_value(decrypted));
        response.set_disabled(found->get_disabled());
        if (!found->get_metadata().empty()) {
            response.set_metadata(m_support.secret_value(found->get_metadata()));
        }
        if (!found->get_status().empty()) {
            response.set_status(m_support.connector_status_value(found->get_status()));
        }

        m_context.metrics().record_flow("connector_retrieve");
        m_context.metrics().counter("admin.connector_retrieve").increment();
        return ConnectorRetrieveResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
