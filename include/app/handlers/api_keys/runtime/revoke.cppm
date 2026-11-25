export module app_handlers_api_keys:runtime_revoke;

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
import core_database;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::api_keys {

class ApiKeyRevokeRuntime
{
public:
    explicit ApiKeyRevokeRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief core::api_keys::revoke_api_key (436-497). */
    [[nodiscard]] std::expected<ApiKeyRevokeResult, core::error::AppError>
    revoke(const ApiKeyRevokeInput& input) const
    {
        const auto found = m_support.find_api_key(input.merchant_id, input.key_id);
        if (!found) {
            return std::unexpected{m_support.api_key_not_found_error()};
        }

        app::store::api_keys::ApiKeyRecord record = *found;
        record.set_revoked(true);
        if (!m_context.repository().update_api_key(record)) {
            return std::unexpected{m_support.api_key_not_found_error()};
        }

        m_support.upsert_expiry_record(record, std::nullopt);

        // decision REVOKE (core/api_keys.rs:453-464).
        m_support.revoke_api_key_decision(record.get_api_key());

        if constexpr (kEmail) {
            if (auto expiry_error = m_support.revoke_api_key_expiry_task(input.key_id)) {
                return std::unexpected{*expiry_error};
            }
        }

        m_context.metrics().record_flow("ApiKeyRevoke");
        m_context.metrics().counter("API_KEY_REVOKED").increment();
        return ApiKeyRevokeResult{m_support.build_revoke_response(record)};
    }

private:
    app::context::AppContext& m_context;
    ApiKeysRuntimeSupport m_support;
};

} // namespace app::handlers::api_keys
