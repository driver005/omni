export module app_routing:routing_handlers_v1_migration;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_model;
import app_context;
import core_error;

import :routing_input;
import :routing_result;
import :routing_runtime;

export namespace app::handlers::routing {

/**
 * v1 migration family: rule migration and migration status (AdminApiAuth).
 * Owns its own add_route registrations; the v1 version facade replays
 * install_route_NN in the original order. Runtime/input support are owned
 * once by the root facade and shared by ref.
 */
class RoutingV1MigrationHandlers
{
public:
    RoutingV1MigrationHandlers(
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

    // ─── routing_migration_status (core:3457) ───────────────────────────────────────
    void handle_routing_migration_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "routing_migration_status", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        if (const auto body = req.body_as_string(); !body.empty()) {
            auto typed = core::behavior::Serializer<
                ::app::model::routing::RoutingMigrationStatusQuery>::pass(req, sink);
            if (!typed) {
                return;
            }
        }
        m_runtime.routing_migration_status(m_request_support.context(req)).handle(sink);
    }

    // ─── migrate_rules_for_profiles (core:3072) + migrate_rules_for_profile (:3193) ─
    void handle_migrate_routing_rules(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "migrate_routing_rules", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<::app::model::routing::RuleMigrationRequest>::pass(
            req,
            sink
        );
        if (!parsed) {
            return;
        }
        m_runtime.migrate_routing_rules(*parsed, m_request_support.context(req)).handle(sink);
    }

    // ─── Route registrations (original install() order) ────────────────────────────
    void install_route_7(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "rule"}.add_route(
                Route{"migrate"}
                    .post(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_migrate_routing_rules(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch migrate_routing_rules")
            )
        );
    }

    void install_route_8(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "migration"}.add_route(
                Route{"status"}
                    .get(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_routing_migration_status(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch routing_migration_status")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_7(router);
        install_route_8(router);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntime& m_runtime;
    RoutingInputSupport& m_input_support;
    RoutingRequestSupport& m_request_support;
};

} // namespace app::handlers::routing
