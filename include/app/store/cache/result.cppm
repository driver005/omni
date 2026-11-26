export module app_store:cache_result;

import std;

export namespace app::handlers::cache {

/**
 * @brief Typed result marker for upstream ApplicationResponse::StatusOk.
 *
 * core/cache.rs:20 and services/api.rs:499 -> http_response_ok(): HTTP 200 with
 * an empty body. There is no payload to serialize; the handler writes an empty
 * body. Kept as a distinct result type so the runtime contract stays typed.
 */
struct CacheInvalidateResult
{};

} // namespace app::handlers::cache
