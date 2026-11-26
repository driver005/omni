export module app_user:admin_handlers_v2_kv;

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

class AdminHandlersV2Kv
{
public:
    AdminHandlersV2Kv(
        ::app::context::AppContext& context,
        AdminRuntime& runtime,
        AdminInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_merchant_account_toggle_kv(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto error = m_input_support.authorize(req, "merchant_account_toggle_kv")) {
            res.set_status(error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *error));
            send();
            return;
        }
        const std::string merchant_id{m_input_support.path_part(req, 2)};
        if (merchant_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant id"
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
        auto parsed =
            serde::Ser::deserialize<::app::dto::admin::ToggleKVRequest>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        MerchantAccountToggleKvInput input{merchant_id, parsed->get_kv_enabled()};
        auto result = m_runtime.toggle_kv(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_merchant_account_kv_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto error = m_input_support.authorize(req, "merchant_account_kv_status")) {
            res.set_status(error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *error));
            send();
            return;
        }
        const std::string merchant_id{m_input_support.path_part(req, 2)};
        if (merchant_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant id"
            );
            error.send(res, send);
            return;
        }
        MerchantAccountKvStatusInput input{merchant_id};
        auto result = m_runtime.kv_status(input);
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
};

} // namespace app::handlers::admin
