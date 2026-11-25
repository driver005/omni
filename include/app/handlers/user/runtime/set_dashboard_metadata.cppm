export module app_handlers_user:runtime_set_dashboard_metadata;

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
import app_connector;
import app_events;
import app_workflow;
import app_keystore;
import app_locking;
import app_services;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::user {

class SetDashboardMetadataRuntime
{
public:
    explicit SetDashboardMetadataRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<SetDashboardMetadataResult, core::error::AppError>
    set_dashboard_metadata(const SetDashboardMetadataInput& input) const
    {
        std::string set_cookie{};


        const auto merchant_id = input.request_context.merchant_id;
        const auto client_ip = input.request_context.client_ip;
        const auto normalized = std::format(
            R"({{"ip":"{}","data":{}}})",
            std::string{client_ip.empty() ? std::string_view{"unknown"} : client_ip},
            input.request_context.body
        );
        m_context.repository().set_config("dashboard_metadata." + merchant_id, normalized);

        m_context.metrics().record_flow("set_dashboard_metadata");
        m_context.metrics().counter("user.set_dashboard_metadata").increment();

        return SetDashboardMetadataResult{ResponseDocument::empty(), set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
