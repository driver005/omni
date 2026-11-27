export module app_user:admin_handlers_v1_webhooks;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_behavior;
import core_error;
import app_http;
import :admin_input;
import :admin_result;
import :admin_runtime;

export namespace app::handlers::admin {

class AdminHandlersV1Webhooks
{
public:
    AdminHandlersV1Webhooks(
        ::app::context::AppContext& context,
        AdminRuntime& runtime,
        AdminInputSupport& input_support,
        AdminRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_support{request_support}
    {
    }

    void handle_connector_webhook_register(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "connector_webhook_register", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const std::string connector_id{
            m_support.path(req, AdminPolicy::path_index(req, 1))
        };
        if (connector_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing connector id"
            );
            error.send(sink);
            return;
        }
        std::string merchant_id{::app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && m_input_support.path_has(req, "account")) {
            merchant_id = m_support.path(req, AdminPolicy::path_index(req, 4));
        }
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<::app::dto::admin::WebhookDetailsRequest>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{std::move(merchant_id)});
        ctx.claims.insert_or_assign("connector_id", std::string{connector_id});
        m_runtime.register_webhook(*parsed, ctx).handle(sink);
    }

    void handle_retrieve_connector_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "retrieve_connector_webhook", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const std::string connector_id{
            m_support.path(req, AdminPolicy::path_index(req, 1))
        };
        if (connector_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing connector id"
            );
            error.send(sink);
            return;
        }
        std::string merchant_id{::app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && m_input_support.path_has(req, "account")) {
            merchant_id = m_support.path(req, AdminPolicy::path_index(req, 4));
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{std::move(merchant_id)});
        ctx.claims.insert_or_assign("connector_id", std::string{connector_id});
        m_runtime.fetch_webhook(ctx).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // routes/app.rs:2233-2273 (MerchantConnectorAccount::server, v1).
        Router{router, "account"}
            .add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "connectors"}.add_router(
                        Router{router, "webhooks"}.add_route(
                            Route{":merchant_connector_id"}
                                .post(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        handle_connector_webhook_register(
                                            req,
                                            res,
                                            std::move(send)
                                        );
                                    }
                                )
                                .summary("Hyperswitch connector_webhook_register")
                        )
                    )
                )
            )
            .add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "connectors"}.add_router(
                        Router{router, "webhooks"}.add_route(
                            Route{":merchant_connector_id"}
                                .get(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        handle_retrieve_connector_webhook(
                                            req,
                                            res,
                                            std::move(send)
                                        );
                                    }
                                )
                                .summary("Hyperswitch retrieve_connector_webhook")
                        )
                    )
                )
            );
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntime& m_runtime;
    AdminInputSupport& m_input_support;
    AdminRequestSupport& m_support;
};

} // namespace app::handlers::admin
