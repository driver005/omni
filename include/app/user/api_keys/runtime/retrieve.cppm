export module app_user:api_keys_runtime_retrieve;

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

class ApiKeyRetrieveRuntime
{
public:
    explicit ApiKeyRetrieveRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief core::api_keys::retrieve_api_key (261-275). */
    [[nodiscard]] core::behavior::Outcome<::app::dto::api_keys::RetrieveApiKeyResponse>
    retrieve(const ::core::behavior::CallContext& ctx) const
    {
        const auto key_id = ctx.path_segments.size() > 2 ? ctx.path_segments[2] : std::string{};
        const auto found = m_support.find_api_key(ctx.merchant_id, key_id);
        if (!found) {
            return std::unexpected{m_support.api_key_not_found_error()};
        }

        m_context.logger().record_flow("ApiKeyRetrieve");
        m_context.logger().counter("API_KEY_RETRIEVED").increment();
        return m_support.to_retrieve_response(*found);
    }

private:
    ::app::context::AppContext& m_context;
    ApiKeysRuntimeSupport m_support;
};

} // namespace app::handlers::api_keys
