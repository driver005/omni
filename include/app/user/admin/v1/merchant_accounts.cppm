export module app_user:admin_handlers_v1_merchant_accounts;

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

class AdminHandlersV1MerchantAccounts
{
public:
    AdminHandlersV1MerchantAccounts(
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

    void handle_merchant_account_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "merchant_account_create", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::dto::admin::MerchantAccountCreate>::pass(req, sink);
        if (!parsed) {
            return;
        }
        std::string organization_id{::app::http::org_id_from_header{}(req)};
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("organization_id", std::string{std::move(organization_id)});
        m_runtime.create_merchant_account(*parsed, ctx).handle(sink);
    }

    void handle_merchant_account_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "merchant_account_list", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        std::string organization_id{
            ::app::http::RequestQuery{}(req).get("organization_id").value_or("")
        };
        if (organization_id.empty() && m_input_support.path_has(req, "organizations")) {
            organization_id = m_support.path(req, AdminPolicy::path_index(req, 2));
        }
        const std::string merchant_filter{
            ::app::http::RequestQuery{}(req).get("merchant_id").value_or("")
        };
        const std::size_t offset{m_input_support.size_query(req, "offset", 0)};
        const std::size_t limit{m_input_support.size_query(req, "limit", 0)};
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("organization_id", std::string{std::move(organization_id)});
        ctx.claims.insert_or_assign("merchant_filter", std::string{merchant_filter});
        ctx.claims.insert_or_assign("offset", std::to_string(offset));
        ctx.claims.insert_or_assign("limit", std::to_string(limit));
        m_runtime.list_merchant_accounts(ctx).handle(sink);
    }

    void handle_merchant_account_transfer_keys(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "merchant_account_transfer_keys", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::dto::admin::MerchantKeyTransferRequest>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("offset", std::to_string(parsed->get_from()));
        ctx.claims.insert_or_assign("limit", std::to_string(parsed->get_limit()));
        m_runtime.transfer_keys(ctx).handle(sink);
    }

    void handle_retrieve_merchant_account(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "retrieve_merchant_account", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const std::string merchant_id{m_support.path(req, AdminPolicy::path_index(req, 1))};
        if (merchant_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant id"
            );
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{merchant_id});
        m_runtime.retrieve_merchant_account(ctx).handle(sink);
    }

    void handle_update_merchant_account(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "update_merchant_account", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const std::string merchant_id{m_support.path(req, AdminPolicy::path_index(req, 1))};
        if (merchant_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant id"
            );
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::dto::admin::MerchantAccountUpdate>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{merchant_id});
        m_runtime.update_merchant_account(*parsed, ctx).handle(sink);
    }

    void handle_delete_merchant_account(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "delete_merchant_account", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const std::string merchant_id{m_support.path(req, AdminPolicy::path_index(req, 1))};
        if (merchant_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant id"
            );
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{merchant_id});
        m_runtime.delete_merchant_account(ctx).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // routes/app.rs:2181-2207 (MerchantAccount::server, v1).
        Router{router, "accounts"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_merchant_account_create(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch merchant_account_create")
            )
            .add_route(
                Route{"list"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_merchant_account_list(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch merchant_account_list")
            );
    }

    // Interleaved chain fragment — called from the version facade in original order.
    void install_middle(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // routes/app.rs:2181-2207 (MerchantAccount::server, v1).
        Router{router, "accounts"}.add_route(
            Route{"transfer"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_merchant_account_transfer_keys(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch merchant_account_transfer_keys")
        );
    }

    // Interleaved chain fragment — called from the version facade in original order.
    void install_tail(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // routes/app.rs:2181-2207 (MerchantAccount::server, v1).
        Router{router, "accounts"}
            .add_route(
                Route{":id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_retrieve_merchant_account(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch retrieve_merchant_account")
            )
            .add_route(
                Route{":id"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_update_merchant_account(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch update_merchant_account")
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
                            handle_delete_merchant_account(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch delete_merchant_account")
            );
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntime& m_runtime;
    AdminInputSupport& m_input_support;
    AdminRequestSupport& m_support;
};

} // namespace app::handlers::admin
