export module app_handlers_api_keys:runtime_create;

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

class ApiKeyCreateRuntime
{
public:
    explicit ApiKeyCreateRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief core::api_keys::create_api_key (112-189). */
    [[nodiscard]] std::expected<ApiKeyCreateResult, core::error::AppError>
    create(const ApiKeyCreateInput& input) const
    {
        auto hash_key = m_support.get_hash_key();
        if (!hash_key) {
            return std::unexpected{hash_key.error()};
        }

        const std::string env = m_support.env_prefix();
        const PlaintextApiKey plaintext = m_support.new_plaintext_api_key(kApiKeyLength, env);
        const std::string key_id = m_support.new_key_id(env);
        const HashedApiKey hashed = m_support.keyed_hash(plaintext, hash_key->value());
        const std::int64_t created_epoch = m_support.now_epoch();
        const std::string created_at = m_support.iso_utc(created_epoch);
        const std::optional<std::string> expires =
            m_support.expires_at(input.request.get_expiration());

        app::store::api_keys::ApiKeyRecord record;
        record.set_key_id(key_id);
        record.set_merchant_id(input.merchant_id);
        record.set_name(input.request.get_name());
        record.set_description(input.request.get_description().value_or(""));
        record.set_prefix(plaintext.prefix());
        record.set_api_key(hashed.peek());
        record.set_expiration(m_support.expiration_string(input.request.get_expiration()));
        record.set_created_at(created_at);
        record.set_revoked(false);

        auto inserted = m_context.repository().create_api_key(std::move(record));
        if (!inserted) {
            return std::unexpected{inserted.error()};
        }
        const app::store::api_keys::ApiKeyRecord saved = *inserted;

        m_support.upsert_expiry_record(saved, expires);

        // decision ADD tracked job (core/api_keys.rs:148-160).
        m_support.add_api_key_decision(
            saved.get_api_key(),
            saved.get_merchant_id(),
            saved.get_key_id(),
            expires
        );

        // metrics::API_KEY_CREATED.add(1, merchant attribute) (core:162-165).
        m_context.metrics().record_flow("ApiKeyCreate");
        m_context.metrics().counter("API_KEY_CREATED").increment();

        if constexpr (kEmail) {
            if (expires) {
                if (auto expiry_error = m_support.add_api_key_expiry_task(saved, expires)) {
                    return std::unexpected{*expiry_error};
                }
            }
        }

        return ApiKeyCreateResult{m_support.build_create_response(saved, plaintext, input.request)};
    }

private:
    app::context::AppContext& m_context;
    ApiKeysRuntimeSupport m_support;
};

} // namespace app::handlers::api_keys
