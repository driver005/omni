export module app_handlers_revenue_recovery_redis:runtime_common;

import std;
import core_serde;
import core_cache;
import core_error;
import app_context;

export namespace app::handlers::revenue_recovery_redis {

/**
 * @brief Output of the ported `RedisTokenManager::get_redis_key_data_raw`.
 *
 * Mirrors the upstream tuple `(bool, i64, Option<serde_json::Value>)`:
 * `exists` (key/hash presence), `ttl_seconds` (Redis TTL), `data` (the raw
 * Redis value materialized as a JSON value).
 *
 * Upstream: types/storage/revenue_recovery_redis_operation.rs:1073-1146.
 */
struct RawRedisData
{
    bool exists{false};
    std::int64_t ttl_seconds{-2};
    serde::Value data{};
};

/**
 * @brief In-process support for the Redis substitute used by the route runtime.
 */
class RevenueRecoveryRedisRuntimeSupport
{
public:
    explicit RevenueRecoveryRedisRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /**
     * @brief Local, in-process port of `RedisTokenManager::get_redis_key_data_raw`.
     *
     * Upstream: types/storage/revenue_recovery_redis_operation.rs:1073-1146.
     * The cache entry is the substitute for the Redis key; expiry carries the TTL
     * and the raw string value carries the Redis value (a plain string for the
     * Status key, a JSON object for the Tokens hash).
     *
     * Behavior ported line-by-line:
     *  - TTL (:1091-1098): Redis TTL semantics -2 = missing, -1 = no expiry,
     *    otherwise remaining whole seconds.
     *  - Status (:1101-1114): get_key::<String> Ok -> (true, Value::String(v));
     *    Err -> (false, Value::String("Error retrieving status key: <err>")), where
     *    <err> is RedisError::GetFailed's display "Failed to get key value in Redis"
     *    (redis_interface/src/errors.rs:20-21).
     *  - Tokens (:1115-1134): get_hash_fields::<HashMap<String,String>> Ok ->
     *    exists = !map.is_empty(), data = Value(map) or Value::Object({}); Err ->
     *    (false, Value::Null). An absent key makes HGETALL return an empty map,
     *    i.e. Ok(empty) -> (false, Value::Object({})).
     */
    [[nodiscard]] RawRedisData get_redis_key_data_raw(
        std::string_view cache_key,
        bool is_tokens,
        std::string_view customer_id
    ) const
    {
        RawRedisData out;

        // Locate the value entry. For the Status lock key also look at the
        // producer's lock-registry key (app/locking/lock_registry.cppm:57-60,
        // revenue_recovery_data_backfill.cppm:442-445) so the substitute reflects
        // a lock actually acquired by the backfill flow. The lock registry stores
        // the owner (payment id) as a plain string, exactly the upstream Status
        // value (revenue_recovery_redis_operation.rs:162-168).
        std::optional<core::cache::CacheEntry> entry = find_entry(cache_key);
        if (!is_tokens && !entry) {
            const auto lock_key =
                std::string{"lock:revenue_recovery.status:"} + std::string{customer_id};
            entry = find_entry(lock_key);
        }
        const core::cache::CacheEntry* value_entry = entry ? &*entry : nullptr;

        // TTL (revenue_recovery_redis_operation.rs:1091-1098).
        out.ttl_seconds = -2;
        if (value_entry != nullptr) {
            if (value_entry->expires_at == core::cache::CacheEntry::Clock::time_point::max()) {
                out.ttl_seconds = -1;
            } else {
                out.ttl_seconds = std::max<std::int64_t>(
                    0,
                    std::chrono::duration_cast<std::chrono::seconds>(
                        value_entry->expires_at - core::cache::CacheEntry::Clock::now()
                    )
                        .count()
                );
            }
        }

        if (!is_tokens) {
            // Status: get_key::<String> (:1102-1114).
            if (value_entry != nullptr && !core::cache::IsMiss{}(value_entry->value)) {
                auto text = core::cache::value_as<std::string>(value_entry->value, std::string{});
                // A test/producer may store the value JSON-quoted; unwrap it so it
                // matches the plain String redis returns from GET.
                if (auto decoded = serde::Ser::decode_generic("application/json", text)) {
                    if (const auto* scalar = std::get_if<std::string>(&decoded->get())) {
                        text = *scalar;
                    }
                }
                out.exists = true;
                out.data = serde::Value{std::move(text)};
            } else {
                out.exists = false;
                out.data = serde::Value{
                    std::string{"Error retrieving status key: Failed to get key value in Redis"}
                };
            }
            return out;
        }

        // Tokens: get_hash_fields::<HashMap<String,String>> (:1115-1134).
        if (value_entry == nullptr) {
            // HGETALL on a missing key -> empty map -> Ok(empty).
            out.exists = false;
            out.data = serde::Value{serde::Value::Object{}};
            return out;
        }
        if (core::cache::IsMiss{}(value_entry->value)) {
            out.exists = false;
            out.data = serde::Value{serde::Value::Object{}};
            return out;
        }
        const auto raw = core::cache::value_as<std::string>(value_entry->value, std::string{});
        const auto decoded = serde::Ser::decode_generic("application/json", raw);
        if (!decoded) {
            // Hash decode failure mirrors get_hash_fields Err -> (false, Value::Null).
            out.exists = false;
            out.data = serde::Value{};
            return out;
        }
        const auto* object = std::get_if<serde::Value::Object>(&decoded->get());
        if (object == nullptr) {
            out.exists = false;
            out.data = serde::Value{};
            return out;
        }
        out.exists = !object->empty();
        out.data = serde::Value{*object};
        return out;
    }

    [[nodiscard]] std::optional<core::cache::CacheEntry> find_entry(std::string_view key) const
    {
        core::cache::CacheQuerySpec spec;
        spec.prefix = std::string{key};
        for (const auto& entry: m_context.cache().execute(spec)) {
            if (entry.key == key) {
                return entry;
            }
        }
        return std::nullopt;
    }

    // GenericNotFoundError mapping from core/revenue_recovery_data_backfill.rs:146-154.
    [[nodiscard]] core::error::AppError
    not_found_error(std::string_view connector_customer_id) const
    {
        const auto not_found_message =
            std::string{"Redis data not found for connector customer id:- '"} +
            std::string{connector_customer_id} + "'";
        return core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::NOT_FOUND,
            not_found_message
        );
    }

private:
    app::context::AppContext& m_context;
};

} // namespace app::handlers::revenue_recovery_redis
