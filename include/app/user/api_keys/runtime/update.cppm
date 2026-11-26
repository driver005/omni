export module app_user:api_keys_runtime_update;

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

import :api_keys_input;
import :api_keys_result;
import :api_keys_runtime_common;

export namespace app::handlers::api_keys {

class ApiKeyUpdateRuntime
{
public:
    explicit ApiKeyUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief core::api_keys::update_api_key (278-369). */
    [[nodiscard]] std::expected<ApiKeyUpdateResult, core::error::AppError>
    update(const ApiKeyUpdateInput& input) const
    {
        const auto found = m_support.find_api_key(input.merchant_id, input.key_id);
        if (!found) {
            return std::unexpected{m_support.api_key_not_found_error()};
        }

        ::app::store::api_keys::ApiKeyRecord record = *found;
        if (input.request.get_name()) {
            record.set_name(*input.request.get_name());
        }
        if (input.request.get_description()) {
            record.set_description(*input.request.get_description());
        }
        const bool expiration_changed = input.request.get_expiration().has_value();
        if (expiration_changed) {
            record.set_expiration(m_support.expiration_string(*input.request.get_expiration()));
        }
        record.set_key_id(input.key_id);
        record.set_merchant_id(input.merchant_id);
        if (!m_context.repository().update_api_key(record)) {
            return std::unexpected{m_support.api_key_not_found_error()};
        }

        const ::app::dto::api_keys::ApiKeyExpiration effective =
            expiration_changed ? *input.request.get_expiration()
                               : m_support.expiration_enum(record.get_expiration());
        const std::optional<std::string> expires = m_support.expires_at(effective);
        m_support.upsert_expiry_record(record, expires);

        // decision ADD on update (core/api_keys.rs:300-312).
        m_support
            .add_api_key_decision(record.get_api_key(), input.merchant_id, input.key_id, expires);

        if constexpr (kEmail) {
            if (auto expiry_error = m_support.update_api_key_expiry_task(record, expires)) {
                return std::unexpected{*expiry_error};
            }
        }

        m_context.metrics().record_flow("ApiKeyUpdate");
        m_context.metrics().counter("API_KEY_UPDATED").increment();
        return ApiKeyUpdateResult{m_support.to_retrieve_response(record)};
    }

private:
    ::app::context::AppContext& m_context;
    ApiKeysRuntimeSupport m_support;
};

} // namespace app::handlers::api_keys
