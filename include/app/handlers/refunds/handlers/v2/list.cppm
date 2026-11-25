export module app_handlers_refunds:handlers_v2_list;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_context;
import core_error;
import app_auth;
import app_http;
import app_locking;

import :input;
import :result;
import :runtime;

export namespace app::handlers::refunds {

class RefundsV2ListHandlers
{
public:
    RefundsV2ListHandlers(
        app::context::AppContext& ctx,
        RefundsRuntime& runtime,
        RefundsInputSupport& input_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    /** @brief refunds_list — routes/refunds.rs:502-531 (v1), :535-569 (v2). */
    void handle_refunds_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_op(
                req,
                m_input_support.is_v2(req) ? AuthProfile::V2List : AuthProfile::V1List
            )) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = app::model::refunds::RefundListRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const bool profile_scoped = m_input_support.is_v2(req) ||
                                    req.get_path().find("/profile/list") != std::string_view::npos;
        RefundsListInput input{*parsed, profile_scoped, m_input_support.request_facts(req)};

        auto result = m_runtime.list(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // original install() route #20
    void install_route_20(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/refunds"}.add_route(
            Route{"list"}
                .post(
                    [this](auto& rq, auto& rs, auto sd)
                    {
                        handle_refunds_list(rq, rs, std::move(sd));
                    }
                )
                .summary("Hyperswitch refunds_list")
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_20(router);
    }

private:
    app::context::AppContext& m_context;
    RefundsRuntime& m_runtime;
    RefundsInputSupport& m_input_support;
};

} // namespace app::handlers::refunds
