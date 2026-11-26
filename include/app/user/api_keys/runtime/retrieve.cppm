export module app_user:api_keys_runtime_retrieve;

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

class ApiKeyRetrieveRuntime
{
public:
    explicit ApiKeyRetrieveRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief core::api_keys::retrieve_api_key (261-275). */
    [[nodiscard]] std::expected<ApiKeyRetrieveResult, core::error::AppError>
    retrieve(const ApiKeyRetrieveInput& input) const
    {
        const auto found = m_support.find_api_key(input.merchant_id, input.key_id);
        if (!found) {
            return std::unexpected{m_support.api_key_not_found_error()};
        }

        m_context.metrics().record_flow("ApiKeyRetrieve");
        m_context.metrics().counter("API_KEY_RETRIEVED").increment();
        return ApiKeyRetrieveResult{m_support.to_retrieve_response(*found)};
    }

private:
    ::app::context::AppContext& m_context;
    ApiKeysRuntimeSupport m_support;
};

} // namespace app::handlers::api_keys
