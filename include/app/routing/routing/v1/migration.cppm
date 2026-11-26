export module app_routing:routing_handlers_v1_migration;

import std;
import interfaces;
import core_router;
import core_serde;
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
        RoutingInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    // ─── routing_migration_status (core:3457) ───────────────────────────────────────
    void handle_routing_migration_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_input_support.authorize_operation(req, "routing_migration_status")) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        RoutingInput input{};
        m_input_support.fill_common(req, input);
        input.migration_limit =
            m_input_support.query_number<std::uint32_t>(req, "limit").value_or(20);
        input.migration_offset =
            m_input_support.query_number<std::uint32_t>(req, "offset").value_or(0);
        if (const auto body = req.body_as_string(); !body.empty()) {
            auto typed =
                serde::Ser::deserialize<::app::model::routing::RoutingMigrationStatusQuery>(
                    "application/json",
                    body
                );
            if (!typed) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_JSON,
                    "invalid JSON"
                );
                error.send(res, send);
                return;
            }
        }

        auto result = m_runtime.routing_migration_status(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    // ─── migrate_rules_for_profiles (core:3072) + migrate_rules_for_profile (:3193) ─
    void handle_migrate_routing_rules(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_operation(req, "migrate_routing_rules")) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        auto typed = serde::Ser::deserialize<::app::model::routing::RuleMigrationRequest>(
            "application/json",
            body
        );
        if (!typed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        RoutingInput input{};
        m_input_support.fill_common(req, input);
        input.body = body;
        input.migration_request = *typed;

        auto result = m_runtime.migrate_routing_rules(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
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
};

} // namespace app::handlers::routing
