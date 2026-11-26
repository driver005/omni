export module app_user:admin_runtime_connector_list;

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

class ConnectorListRuntime
{
public:
    explicit ConnectorListRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ConnectorListResult, core::error::AppError>
    list(const ConnectorListInput& input) const
    {
        std::vector<::app::dto::admin::MerchantConnectorListResponse> response;
        if (input.for_profile) {
            response = m_support.list_connectors_for_a_profile(input.profile_id_filter);
        } else {
            response =
                m_support.list_payment_connectors(input.merchant_id, input.profile_id_filter);
        }

        m_context.metrics().record_flow("connector_list");
        m_context.metrics().counter("admin.connector_list").increment();
        return ConnectorListResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
