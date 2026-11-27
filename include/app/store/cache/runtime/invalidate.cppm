export module app_store:cache_runtime_invalidate;

import std;
import core_behavior;
import core_error;
import app_context;
import app_metrics;

import :cache_runtime_common;

export namespace app::handlers::cache {

class InvalidateRuntime
{
public:
    explicit InvalidateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Exact port of core::cache::invalidate (core/cache.rs:8-24):
    //   redact_from_redis_and_publish(store, [CacheKind::All(key)]) -> result;
    //   if result > 0 -> StatusOk, else -> InternalServerError.
    // A zero-delete Redis reply is only logged upstream (cache.rs:988-998),
    // never a failure; a cache miss is therefore not the success gate.
    [[nodiscard]] core::behavior::Outcome<void>
    invalidate(const core::behavior::CallContext& ctx) const
    {
        // Flow::CacheInvalidate instrumentation: upstream records this through
        // server_wrap (routes/cache.rs:16,20-28). The handler owns the auth gate;
        // this runtime only performs the cache invalidation itself.
        m_context.metrics().record_flow("CacheInvalidate");

        // web::Path<String> (/cache/invalidate/:key). Longer mount prefixes fall
        // back to the last non-empty segment, matching the original helper.
        std::string key{};
        if (ctx.path_segments.size() > 2U) {
            key = ctx.path_segments[2U];
        }
        if (key.empty() && !ctx.path_segments.empty()) {
            key = ctx.path_segments.back();
        }

        const std::size_t published = m_support.redact_from_redis_and_publish(key);
        if (published == 0) {
            // core/cache.rs:21-24: InternalServerError with the printable
            // attachment "Failed to invalidate cache". That is the real
            // human-readable message; upstream code HE_00 goes in kind_name.
            auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INTERNAL,
                "Failed to invalidate cache"
            );
            error.set_kind_name("HE_00");
            return std::unexpected{std::move(error)};
        }

        m_context.metrics().counter("CACHE_INVALIDATED").increment();

        // ApplicationResponse::StatusOk has no body (core/cache.rs:20;
        // services/api.rs:499 -> http_response_ok()).
        return {};
    }

private:
    ::app::context::AppContext& m_context;
    CacheRuntimeSupport m_support;
};

} // namespace app::handlers::cache
