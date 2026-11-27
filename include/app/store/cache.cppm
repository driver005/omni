export module app_store:cache_handler;

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
        // 1. request -> Input (auth/path; no body is read on this route).
        const auto auth_error = m_request_support.authorize(req, "invalidate", {});
        // Upstream takes the key from web::Path<String> (/cache/invalidate/:key).
        auto key = m_request_support.path(req, 2U);
        if (key.empty()) {
            // Longer mount prefixes: fall back to the last segment, matching the
            // old RequestPath-based helper.
            for (std::size_t index{0};; ++index) {
                const auto segment = m_request_support.path(req, index);
                if (segment.empty()) {
                    break;
                }
                key = segment;
            }
        }
        CacheInvalidateInput input{auth_error, key};

        // 2. Input -> Result.
        auto result = m_runtime.invalidate(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        // 3. Result -> response. StatusOk has an empty body; the typed result
        // carries no fields, so no JSON document is written.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::vector<std::byte>{});
        send();
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
