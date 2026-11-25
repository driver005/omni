export module app_handlers_customers:handlers_v2_global_id;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_store;
import app_http;
import app_metrics;

import :input;
import :result;
import :runtime;

export namespace app::handlers::customers {

class CustomersHandlersV2GlobalId
{
public:
    CustomersHandlersV2GlobalId(
        app::context::AppContext& context,
        CustomersRuntime& runtime,
        CustomersInputSupport& input_support
    ) noexcept :
        m_context{&context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_migrate_global_id(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream `V2AdminApiAuth` — local equivalent is the platform-admin rule.
        const auto rule = ::app::auth::RouteAuthRule{
            ::app::auth::AuthScheme::PLATFORM_ADMIN,
            ::app::auth::AppPermission::ADMIN
        };
        if (const auto denied = m_context->authorizer().authorize_route(
                ::app::auth::AuthContextBuilder{}(req),
                rule
            )) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string body = req.body_as_string();
        auto rows = m_input_support.parse_customer_global_id_migration_csv(body);
        if (!rows) {
            const auto& error = rows.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        CustomersMigrateRouteInput input{std::move(*rows)};
        auto result = m_runtime.migrate_global_id(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    /** @brief Owns the `migrate/global-id` sub-router (admin-gated v2 route). */
    [[nodiscard]] utils::openapi::ApiRouter make_router(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        return Router{router, "migrate"}.add_route(
            Route{"global-id"}
                .post(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_migrate_global_id(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch customers migrate_global_id")
        );
    }

private:
    app::context::AppContext* m_context{nullptr};
    CustomersRuntime& m_runtime;
    CustomersInputSupport& m_input_support;
};

} // namespace app::handlers::customers
