export module app_routing:superposition_proxy_handlers_v1;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_context;

import :superposition_proxy_input;
import :superposition_proxy_runtime;
import :superposition_proxy_handlers_v1_contexts;
import :superposition_proxy_handlers_v1_default_configs;
import :superposition_proxy_handlers_v1_dimensions;
import :superposition_proxy_handlers_v1_resolve;
import :superposition_proxy_handlers_v1_audit;

export namespace app::handlers::superposition_proxy {

/**
 * v1 surface: `v1/superposition/...`. Registration stays in one Router chain
 * (one ApiRouter node, one Registry entry, original order) while the thin
 * `handle_*` adapters live in the five family partitions. Runtime and input
 * support are owned by the facade and shared here by reference.
 */
class SuperpositionProxyHandlersV1
{
public:
    SuperpositionProxyHandlersV1(
        ::app::context::AppContext& context,
        SuperpositionProxyRuntime& runtime,
        SuperpositionProxyInputSupport& input_support,
        SuperpositionProxyRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support},
        m_contexts{context, runtime, input_support, request_support},
        m_default_configs{context, runtime, input_support, request_support},
        m_dimensions{context, runtime, input_support, request_support},
        m_resolve{context, runtime, input_support, request_support},
        m_audit{context, runtime, input_support, request_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v1/superposition"}
            .add_route(
                Route{"context"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            m_contexts.handle_list_contexts(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_contexts")
            )
            .add_route(
                Route{"context"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            m_contexts.handle_create_context(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch create_context")
            )
            .add_route(
                Route{"default-config"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            m_default_configs
                                .handle_list_default_configs(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_default_configs")
            )
            .add_router(
                Router{router, "default-config"}.add_route(
                    Route{":key"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                m_default_configs
                                    .handle_get_default_config(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_default_config")
                )
            )
            .add_route(
                Route{"dimension"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            m_dimensions.handle_list_dimensions(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_dimensions")
            )
            .add_router(
                Router{router, "dimension"}.add_route(
                    Route{":dimension_name"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                m_dimensions.handle_get_dimension(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_dimension")
                )
            )
            .add_router(
                Router{router, "config"}.add_router(
                    Router{router, "resolve"}.add_route(
                        Route{"detailed"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    m_resolve
                                        .handle_resolve_detailed_config(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch resolve_detailed_config")
                    )
                )
            )
            .add_router(
                Router{router, "config"}.add_router(
                    Router{router, "resolve"}.add_router(
                        Router{router, "explain"}.add_route(
                            Route{":key"}
                                .post(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        m_resolve.handle_resolve_config_explanation(
                                            req,
                                            res,
                                            std::move(send)
                                        );
                                    }
                                )
                                .summary("Hyperswitch resolve_config_explanation")
                        )
                    )
                )
            )
            .add_route(
                Route{"audit"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            m_audit.handle_list_audit_logs(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_audit_logs")
            );
    }

private:
    ::app::context::AppContext& m_context;
    SuperpositionProxyRuntime& m_runtime;
    SuperpositionProxyInputSupport& m_input_support;
    SuperpositionProxyRequestSupport& m_request_support;
    SuperpositionProxyHandlersV1Contexts m_contexts;
    SuperpositionProxyHandlersV1DefaultConfigs m_default_configs;
    SuperpositionProxyHandlersV1Dimensions m_dimensions;
    SuperpositionProxyHandlersV1Resolve m_resolve;
    SuperpositionProxyHandlersV1Audit m_audit;
};

} // namespace app::handlers::superposition_proxy
