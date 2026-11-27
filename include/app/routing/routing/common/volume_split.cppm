export module app_routing:routing_handlers_volume_split;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_context;
import core_error;

import :routing_input;
import :routing_result;
import :routing_runtime;

export namespace app::handlers::routing {

/**
 * Unversioned volume-split family: set/get dynamic-routing volume split under
 * account/:account_id/business_profile/:profile_id/dynamic_routing.
 * Owns its own add_route registrations; the common facade replays
 * install_route_NN in the original order. Runtime/input support are owned once
 * by the root facade and shared by ref.
 */
class RoutingVolumeSplitHandlers
{
public:
    RoutingVolumeSplitHandlers(
        ::app::context::AppContext& context,
        RoutingRuntime& runtime,
        RoutingInputSupport& input_support,
        RoutingRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support}
    {
    }

    // ─── configure_dynamic_routing_volume_split (core:2624) ─────────────────────────
    void handle_set_dynamic_routing_volume_split(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "set_dynamic_routing_volume_split", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        m_runtime.set_dynamic_routing_volume_split(m_request_support.context(req)).handle(sink);
    }

    // ─── retrieve_dynamic_routing_volume_split (core:2680) ──────────────────────────
    void handle_get_dynamic_routing_volume_split(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "get_dynamic_routing_volume_split", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        m_runtime.get_dynamic_routing_volume_split(m_request_support.context(req)).handle(sink);
    }

    // ─── Route registrations (original install() order) ────────────────────────────
    void install_route_40(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        auto dynamic = Router{router, "dynamic_routing"}.add_route(
            Route{"set_volume_split"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_set_dynamic_routing_volume_split(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch set_dynamic_routing_volume_split")
        );
        auto profile = Router{router, ":profile_id"}.add_router(std::move(dynamic));
        auto business = Router{router, "business_profile"}.add_router(std::move(profile));
        auto account_id = Router{router, ":account_id"}.add_router(std::move(business));
        auto account = Router{router, "account"}.add_router(std::move(account_id));
        (void)Router{router, "routing"}.add_router(std::move(account));
    }

    void install_route_41(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        auto dynamic = Router{router, "dynamic_routing"}.add_route(
            Route{"get_volume_split"}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_get_dynamic_routing_volume_split(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch get_dynamic_routing_volume_split")
        );
        auto profile = Router{router, ":profile_id"}.add_router(std::move(dynamic));
        auto business = Router{router, "business_profile"}.add_router(std::move(profile));
        auto account_id = Router{router, ":account_id"}.add_router(std::move(business));
        auto account = Router{router, "account"}.add_router(std::move(account_id));
        (void)Router{router, "routing"}.add_router(std::move(account));
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_40(router);
        install_route_41(router);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntime& m_runtime;
    RoutingInputSupport& m_input_support;
    RoutingRequestSupport& m_request_support;
};

} // namespace app::handlers::routing
