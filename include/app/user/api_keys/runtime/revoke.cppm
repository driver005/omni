export module app_user:api_keys_runtime_revoke;

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
import core_database;

import :api_keys_input;
import :api_keys_result;
import :api_keys_runtime_common;

export namespace app::handlers::api_keys {

class ApiKeyRevokeRuntime
{
public:
    explicit ApiKeyRevokeRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief core::api_keys::revoke_api_key (436-497). */
    [[nodiscard]] core::behavior::Outcome<::app::dto::api_keys::RevokeApiKeyResponse>
    revoke(const ::core::behavior::CallContext& ctx) const
    {
        const auto found = m_support.find_api_key(ctx.merchant_id, (ctx.path_segments.size() > 2 ? ctx.path_segments[2] : std::string{}));
        if (!found) {
            return std::unexpected{m_support.api_key_not_found_error()};
        }

        ::app::store::api_keys::ApiKeyRecord record = *found;
        record.set_revoked(true);
        if (!m_context.repository().update_api_key(record)) {
            return std::unexpected{m_support.api_key_not_found_error()};
        }

        m_support.upsert_expiry_record(record, std::nullopt);

        // decision REVOKE (core/api_keys.rs:453-464).
        m_support.revoke_api_key_decision(record.get_api_key());

        if constexpr (kEmail) {
            if (auto expiry_error = m_support.revoke_api_key_expiry_task((ctx.path_segments.size() > 2 ? ctx.path_segments[2] : std::string{}))) {
                return std::unexpected{*expiry_error};
            }
        }

        m_context.metrics().record_flow("ApiKeyRevoke");
        m_context.metrics().counter("API_KEY_REVOKED").increment();
        return m_support.build_revoke_response(record);
    }

private:
    ::app::context::AppContext& m_context;
    ApiKeysRuntimeSupport m_support;
};

} // namespace app::handlers::api_keys
