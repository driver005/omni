export module app_user:admin_handlers_v2_connector_accounts;

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

class AdminHandlersV2ConnectorAccounts
{
public:
    AdminHandlersV2ConnectorAccounts(
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

    void handle_connector_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "connector_create", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        auto parsed =
            core::behavior::Serializer<::app::dto::admin::MerchantConnectorCreate>::pass(req, sink);
        if (!parsed) {
            return;
        }
        std::string merchant_id{::app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && m_input_support.path_has(req, "account")) {
            merchant_id = m_support.path(req, AdminPolicy::path_index(req, 2));
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{std::move(merchant_id)});
        m_runtime.create_connector(*parsed, ctx).handle(sink);
    }

    void handle_connector_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "connector_retrieve", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const std::string connector_id{m_support.path(req, AdminPolicy::path_index(req, 1))};
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
            merchant_id = m_support.path(req, AdminPolicy::path_index(req, 2));
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{std::move(merchant_id)});
        ctx.claims.insert_or_assign("connector_id", std::string{connector_id});
        m_runtime.retrieve_connector(ctx).handle(sink);
    }

    void handle_connector_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "connector_update", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const std::string connector_id{m_support.path(req, AdminPolicy::path_index(req, 1))};
        if (connector_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing connector id"
            );
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::dto::admin::MerchantConnectorUpdate>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("connector_id", std::string{connector_id});
        m_runtime.update_connector(*parsed, ctx).handle(sink);
    }

    void handle_connector_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "connector_delete", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const std::string connector_id{m_support.path(req, AdminPolicy::path_index(req, 1))};
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
            merchant_id = m_support.path(req, AdminPolicy::path_index(req, 2));
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{std::move(merchant_id)});
        ctx.claims.insert_or_assign("connector_id", std::string{connector_id});
        m_runtime.delete_connector(ctx).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // routes/app.rs:2211-2231 (MerchantConnectorAccount::server, v2).
        Router{router, "v2/connector-accounts"}
            .add_route(
                Route{""}
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
            .add_route(
                Route{":id"}
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
            .add_route(
                Route{":id"}
                    .put(
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
            .add_route(
                Route{":id"}
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
            );
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntime& m_runtime;
    AdminInputSupport& m_input_support;
    AdminRequestSupport& m_support;
};

} // namespace app::handlers::admin
