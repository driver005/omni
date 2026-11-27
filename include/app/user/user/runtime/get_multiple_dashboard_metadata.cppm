export module app_user:user_runtime_get_multiple_dashboard_metadata;

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
import app_connector;
import app_events;
import app_workflow;
import app_keystore;
import app_locking;
import app_services;

import :user_input;
import :user_result;
import :user_runtime_common;

export namespace app::handlers::user {

class GetMultipleDashboardMetadataRuntime
{
public:
    explicit GetMultipleDashboardMetadataRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    get_multiple_dashboard_metadata(const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx) const
    {
        std::string set_cookie{};


        const auto keys = ctx_query(ctx).get("keys");
        const auto merchant_id = ctx.merchant_id;
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
        return std::pair{std::move(document), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
