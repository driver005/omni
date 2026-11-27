export module app_user:admin_handlers_v2_kv;

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

class AdminHandlersV2Kv
{
public:
    AdminHandlersV2Kv(
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

    void handle_merchant_account_toggle_kv(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "merchant_account_toggle_kv", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const std::string merchant_id{m_support.path(req, AdminPolicy::path_index(req, 2))};
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
            core::behavior::Serializer<::app::dto::admin::ToggleKVRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{merchant_id});
        ctx.claims.insert_or_assign("kv_enabled", parsed->get_kv_enabled() ? "true" : "false");
        m_runtime.toggle_kv(ctx).handle(sink);
    }

    void handle_merchant_account_kv_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "merchant_account_kv_status", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const std::string merchant_id{m_support.path(req, AdminPolicy::path_index(req, 2))};
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
        m_runtime.kv_status(ctx).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // routes/app.rs:2156-2179 (MerchantAccount::server, v2).
        Router{router, "v2/merchant-accounts"}
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"kv"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_merchant_account_toggle_kv(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch merchant_account_toggle_kv")
                )
            )
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"kv"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_merchant_account_kv_status(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch merchant_account_kv_status")
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
