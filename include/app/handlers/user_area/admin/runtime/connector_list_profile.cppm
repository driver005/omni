export module app_user:admin_runtime_connector_list_profile;

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

class ConnectorListProfileRuntime
{
public:
    explicit ConnectorListProfileRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ConnectorListProfileResult, core::error::AppError>
    list(const ConnectorListProfileInput& input) const
    {
        auto response = m_support.list_payment_connectors(input.merchant_id, "");

        m_context.metrics().record_flow("connector_list_profile");
        m_context.metrics().counter("admin.connector_list_profile").increment();
        return ConnectorListProfileResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
