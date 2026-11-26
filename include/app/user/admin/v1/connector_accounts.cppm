export module app_user:admin_handlers_v1_connector_accounts;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_http;
import :admin_input;
import :admin_result;
import :admin_runtime;

export namespace app::handlers::admin {

class AdminHandlersV1ConnectorAccounts
{
public:
    AdminHandlersV1ConnectorAccounts(
        ::app::context::AppContext& context,
        AdminRuntime& runtime,
        AdminInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_connector_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto error = m_input_support.authorize(req, "connector_create")) {
            res.set_status(error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *error));
            send();
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
        auto parsed = serde::Ser::deserialize<::app::dto::admin::MerchantConnectorCreate>(
            "application/json",
            body
        );
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        std::string merchant_id{::app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && m_input_support.path_has(req, "account")) {
            merchant_id = m_input_support.path_part(req, 2);
        }
        ConnectorCreateInput input{std::move(merchant_id), *parsed};
        auto result = m_runtime.create_connector(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_connector_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto error = m_input_support.authorize(req, "connector_retrieve")) {
            res.set_status(error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *error));
            send();
            return;
        }
        const std::string connector_id{m_input_support.path_part(req, 1)};
        if (connector_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing connector id"
            );
            error.send(res, send);
            return;
        }
        std::string merchant_id{::app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && m_input_support.path_has(req, "account")) {
            merchant_id = m_input_support.path_part(req, 2);
        }
        ConnectorRetrieveInput input{std::move(merchant_id), connector_id};
        auto result = m_runtime.retrieve_connector(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_connector_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto error = m_input_support.authorize(req, "connector_update")) {
            res.set_status(error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *error));
            send();
            return;
        }
        const std::string connector_id{m_input_support.path_part(req, 1)};
        if (connector_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing connector id"
            );
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
        auto parsed = serde::Ser::deserialize<::app::dto::admin::MerchantConnectorUpdate>(
            "application/json",
            body
        );
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        ConnectorUpdateInput input{connector_id, *parsed};
        auto result = m_runtime.update_connector(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_connector_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto error = m_input_support.authorize(req, "connector_delete")) {
            res.set_status(error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *error));
            send();
            return;
        }
        const std::string connector_id{m_input_support.path_part(req, 1)};
        if (connector_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing connector id"
            );
            error.send(res, send);
            return;
        }
        std::string merchant_id{::app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && m_input_support.path_has(req, "account")) {
            merchant_id = m_input_support.path_part(req, 2);
        }
        ConnectorDeleteInput input{std::move(merchant_id), connector_id};
        auto result = m_runtime.delete_connector(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // routes/app.rs:2233-2273 (MerchantConnectorAccount::server, v1).
        Router{router, "account"}.add_router(
            Router{router, ":merchant_id"}.add_route(
                Route{"connectors"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_connector_create(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch connector_create")
            )
        );
    }

    // Interleaved chain fragment — called from the version facade in original order.
    void install_middle(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // routes/app.rs:2233-2273 (MerchantConnectorAccount::server, v1).
        Router{router, "account"}.add_router(
            Router{router, ":merchant_id"}.add_router(
                Router{router, "connectors"}.add_route(
                    Route{":merchant_connector_id"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_connector_retrieve(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch connector_retrieve")
                )
            )
        );
    }

    // Interleaved chain fragment — called from the version facade in original order.
    void install_tail(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // routes/app.rs:2233-2273 (MerchantConnectorAccount::server, v1).
        Router{router, "account"}
            .add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "connectors"}.add_route(
                        Route{":merchant_connector_id"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_connector_update(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch connector_update")
                    )
                )
            )
            .add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "connectors"}.add_route(
                        Route{":merchant_connector_id"}
                            .delt(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_connector_delete(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch connector_delete")
                    )
                )
            );
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntime& m_runtime;
    AdminInputSupport& m_input_support;
};

} // namespace app::handlers::admin
