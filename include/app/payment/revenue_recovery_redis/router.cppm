export module app_payment:revenue_recovery_redis_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :revenue_recovery_redis_input;
import :context;
import :revenue_recovery_redis_result;
import :revenue_recovery_redis_runtime;
import core_behavior;
import core_serde;
import utils_openapi;
import core_error;

export namespace app::handlers::revenue_recovery_redis {

/**
 * Version partition for the v2 revenue-recovery Redis data-backfill routes.
 * Holds the runtime by reference: the single runtime instance is owned by the
 * facade, so shared mutable state is never duplicated across partitions.
 */
class RevenueRecoveryRedisHandlersV2
{
public:
    void bind_request_support(RevenueRecoveryRedisRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    RevenueRecoveryRedisRequestSupport* m_request_support{nullptr};

    void handle_get_revenue_recovery_redis_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        // Upstream route uses V2AdminApiAuth (revenue_recovery_redis.rs:30). The
        // declarative rule in RouteAuthRegistry is PLATFORM_ADMIN / ADMIN; use it
        // instead of building a rule inline so the permission stays in one place.
        const auto auth = m_request_support->authorize(
            req,
            "get_revenue_recovery_redis_data",
            spec_for("get_revenue_recovery_redis_data")
        );
        if (auth) {
            const auto& error = *auth;
            error.send(sink);
            return;
        }

        // Upstream route: /:connector_customer_id with web::Path<String>. The id is
        // the final non-empty path segment, read via RequestPath rather than a manual
        // rfind on get_path (which also failed to strip a trailing query string).
        const auto id = m_input_support.connector_customer_id_from_path(req);
        if (!id.has_value()) {
            const auto& error = id.error();
            error.send(sink);
            return;
        }

        // Upstream input: web::Query<GetRedisDataQuery>.key_type with enum
        // RedisKeyType { Status, Tokens } (api_models:204-213). serde rejects a
        // missing or unknown variant, so do the same instead of silently defaulting
        // to Status.
        const auto is_tokens = m_input_support.key_type_from_query(req);
        if (!is_tokens.has_value()) {
            const auto& error = is_tokens.error();
            error.send(sink);
            return;
        }

        auto result = m_runtime.get_revenue_recovery_redis_data(m_request_support->context(req));
        result.handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/recovery/data-backfill"}.add_router(
            Router{router, "redis-data"}.add_route(
                Route{":connector_cutomer_id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_get_revenue_recovery_redis_data(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_revenue_recovery_redis_data")
            )
        );
    }

    RevenueRecoveryRedisHandlersV2(
        ::app::context::AppContext& ctx,
        RevenueRecoveryRedisRuntime& runtime
    ) noexcept :
        m_context{ctx},
        m_input_support{ctx},
        m_runtime{runtime}
    {
    }

private:
    ::app::context::AppContext& m_context;
    RevenueRecoveryRedisInputSupport m_input_support;
    RevenueRecoveryRedisRuntime& m_runtime;
};

/**
 * Facade for the revenue-recovery Redis route family. Owns the single shared runtime;
 * version partitions receive it by reference and only add their own routes.
 */
class RevenueRecoveryRedisRouteHandlers
{
public:
    explicit RevenueRecoveryRedisRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_v2{context, m_runtime},
        m_input_support{context},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
        m_v2.bind_request_support(m_request_support);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    RevenueRecoveryRedisRuntime m_runtime;
    RevenueRecoveryRedisHandlersV2 m_v2;
    RevenueRecoveryRedisInputSupport m_input_support;
    RevenueRecoveryRedisPolicy m_policy;
    RevenueRecoveryRedisRequestSupport m_request_support;
};

} // namespace app::handlers::revenue_recovery_redis
