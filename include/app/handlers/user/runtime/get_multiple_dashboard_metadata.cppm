export module app_handlers_user:runtime_get_multiple_dashboard_metadata;

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

class GetMultipleDashboardMetadataRuntime
{
public:
    explicit GetMultipleDashboardMetadataRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetMultipleDashboardMetadataResult, core::error::AppError>
    get_multiple_dashboard_metadata(const GetMultipleDashboardMetadataInput& input) const
    {
        std::string set_cookie{};


        const auto keys = input.request_context.query.get("keys");
        const auto merchant_id = input.request_context.merchant_id;
        const auto stored = m_context.repository().get_config("dashboard_metadata." + merchant_id);

        m_context.metrics().record_flow("get_multiple_dashboard_metadata");
        m_context.metrics().counter("user.get_multiple_dashboard_metadata").increment();

        // DB-backed metadata envelope; `keys` is recorded as the projection filter.
        // Fall back to a typed data envelope when no metadata is stored or the
        // stored value cannot be decoded into a document.
        serde::Value document = ResponseDocument::empty();
        auto* object = std::get_if<serde::Value::Object>(&document.get());
        (*object)["data"] = ResponseDocument::empty();
        if (stored) {
            if (const auto parsed = serde::Ser::decode_generic("application/json", *stored)) {
                document = *parsed;
            }
        }
        (void)keys;
        return GetMultipleDashboardMetadataResult{std::move(document), set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
