export module app_store:cache_router;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_context;
import core_error;
import app_context_core;
import app_http;

import :cache_input;
import :cache_runtime;
import :cache_runtime_common;

export namespace app::handlers::cache {

class CacheRouteHandlers
{
public:
    void handle_invalidate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Auth denials map to 401 / IR_01: the status comes from
        // AppErrorKind::UNAUTHORIZED and the upstream code lives in kind_name.
        if (const auto auth_error = m_request_support.authorize(req, "invalidate", {})) {
            auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "API key not provided or invalid API key used"
            );
            error.set_kind_name("IR_01");
            error.send(sink);
            return;
        }

        // web::Path<String> (/cache/invalidate/:key) is read from the context by
        // the runtime. StatusOk carries an empty body (Outcome<void>).
        m_runtime.invalidate(m_request_support.context(req)).handle(sink);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "cache"}.add_router(
            Router{router, "invalidate"}.add_route(
                Route{":key"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_invalidate(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch invalidate")
            )
        );
    }

    explicit CacheRouteHandlers(::app::context::AppContext& context) :
        m_runtime{context},
        m_input_support{context},
        m_policy{m_input_support},
        m_request_support{m_policy}
    {
        // Upstream's process-wide subscriber invalidates the shared Caches for
        // every CacheKind (app.rs:566; pub_sub.rs:34-69,92-133). Register this
        // instance's cache as such a subscriber: on_message rebuilds the
        // CacheKey from the CacheRedact tenant + kind and removes it from every
        // cache selected for that kind (pub_sub.rs:107-123). CacheKind::All
        // selects all caches, so the key is erased from this instance's cache.
        ::app::context::AppContext* ctx = &context;
        const CacheKeyCodec codec{};
        m_subscription = InProcessPubSubBroker::instance().subscribe(
            std::string{CacheKeyCodec::kInvalidationChannel},
            [ctx, codec](std::string_view message)
            {
                if (const auto key = codec.key_from_cache_redact(message); !key.empty()) {
                    ctx->cache().erase(key);
                }
            }
        );
    }

private:
    CacheRuntime m_runtime;
    CacheInputSupport m_input_support;
    CachePolicy m_policy;
    core::behavior::RequestSupport<CachePolicy> m_request_support;
    std::size_t m_subscription{0};
};

} // namespace app::handlers::cache
