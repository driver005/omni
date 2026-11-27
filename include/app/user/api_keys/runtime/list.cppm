export module app_user:api_keys_runtime_list;

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

class ApiKeyListRuntime
{
public:
    explicit ApiKeyListRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief core::api_keys::list_api_keys (523-541): merchant-scoped query with
     * limit/offset pushed into the database query, not an in-memory filter. */
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list(
        const ::app::dto::api_keys::ListApiKeyConstraints& constraints,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        using ApiKeyRecord = ::app::store::api_keys::ApiKeyRecord;
        const std::int64_t limit = constraints.get_limit().value_or(-1);
        const std::int64_t offset = constraints.get_skip().value_or(-1);

        core::db::QuerySpec spec = m_context.database().query<ApiKeyRecord>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "merchant_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{ctx.merchant_id}
            }
        );
        if (limit >= 0) {
            spec.limit_count = static_cast<std::size_t>(limit);
        }
        if (offset >= 0) {
            spec.offset_count = static_cast<std::size_t>(offset);
        }

        const std::vector<ApiKeyRecord> records = m_context.database().collect<ApiKeyRecord>(spec);
        std::vector<::app::dto::api_keys::RetrieveApiKeyResponse> responses{};
        responses.reserve(records.size());
        for (const auto& record: records) {
            responses.push_back(m_support.to_retrieve_response(record));
        }

        m_context.metrics().record_flow("ApiKeyList");
        m_context.metrics().counter("API_KEY_LISTED").increment();
        serde::Value::Array array{};
        for (const auto& response: responses) {
            array.push_back(serde::Serializer<::app::dto::api_keys::RetrieveApiKeyResponse>::
                                build_value(response, kRetrieveApiKeyResponseWireFields));
        }
        return serde::Value{std::move(array)};
    }

private:
    ::app::context::AppContext& m_context;
    ApiKeysRuntimeSupport m_support;
};

} // namespace app::handlers::api_keys
