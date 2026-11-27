export module app_user:admin_runtime_connector_delete;

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

class ConnectorDeleteRuntime
{
public:
    explicit ConnectorDeleteRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::MerchantConnectorDeleteResponse>
        remove(const ::core::behavior::CallContext& ctx) const
    {
        const auto found =
            m_context.repository().find_merchant_connector_by_string(ctx_string(ctx,"connector_id"));
        if (!found.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "Merchant connector account does not exist in our records"
            )};
        }
        if (!ctx_string(ctx,"merchant_id").empty() && found->get_merchant_id() != ctx_string(ctx,"merchant_id")) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                std::format(
                    "Invalid merchant_id {} provided for merchant_connector_account",
                    ctx_string(ctx,"merchant_id")
                )
            )};
        }
        const bool deleted = m_context.repository().remove_merchant_connector(found->get_id());
        ::app::dto::admin::MerchantConnectorDeleteResponse response;
        response.set_merchant_id(m_support.merchant_id_value(found->get_merchant_id()));
        response.set_merchant_connector_id(
            m_support.connector_account_id(found->get_merchant_connector_id())
        );
        response.set_deleted(deleted);

        m_context.metrics().record_flow("connector_delete");
        m_context.metrics().counter("admin.connector_delete").increment();
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
