export module app_payment:refunds_handlers_v1_update_reverse_manual_update;

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

import :refunds_input;
import :refunds_result;
import :refunds_runtime;

export namespace app::handlers::refunds {

class RefundsV1UpdateReverseManualUpdateHandlers
{
public:
    void bind_request_support(RefundsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    RefundsRequestSupport* m_request_support{nullptr};

    RefundsV1UpdateReverseManualUpdateHandlers(
        ::app::context::AppContext& ctx,
        RefundsRuntime& runtime,
        RefundsInputSupport& input_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    /** @brief refunds_update — routes/refunds.rs:400-424 (v1). */
    void handle_refunds_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_request_support->authorize(req, "V1Update", spec_for("V1Update"))) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(res, send);
            return;
        }
        using Input = ::app::model::refunds::RefundUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            error.send(res, send);
            return;
        }

        // routes/refunds.rs:408 — path wins over body.
        if (const auto id = m_input_support.refund_id_from_path(req)) {
            parsed->set_refund_id(*id);
        }
        RefundsUpdateInput input{*parsed};

        auto result = m_runtime.update(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    /** @brief refunds_reverse — routes/refunds.rs:431-456 (v1). */
    void handle_refunds_reverse(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_request_support->authorize(req, "V1Reverse", spec_for("V1Reverse"))) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(res, send);
            return;
        }
        using Input = ::app::model::refunds::RefundReverseRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            error.send(res, send);
            return;
        }
        if (const auto id = m_input_support.refund_id_from_path(req)) {
            parsed->set_refund_id(*id);
        }
        RefundsReverseInput input{*parsed};

        auto result = m_runtime.reverse(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    /** @brief refunds_manual_update — routes/refunds.rs:828-847 (AdminApiAuth). */
    void handle_refunds_manual_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support->authorize(req, "Admin", spec_for("Admin"))) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(res, send);
            return;
        }
        using Input = ::app::model::refunds::RefundManualUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            error.send(res, send);
            return;
        }
        if (const auto id = m_input_support.refund_id_from_path(req)) {
            parsed->set_refund_id(*id);
        }
        RefundsManualUpdateInput input{*parsed};

        auto result = m_runtime.manual_update(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        // core/refunds.rs:2277 — ApplicationResponse::StatusOk (200, empty body).
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    // original install() route #4
    void install_route_4(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_route(
            Route{":id"}
                .post(
                    [this](auto& rq, auto& rs, auto sd)
                    {
                        handle_refunds_update(rq, rs, std::move(sd));
                    }
                )
                .summary("Hyperswitch refunds_update")
        );
    }

    // original install() route #5
    void install_route_5(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_router(
            Router{router, ":id"}.add_route(
                Route{"reverse"}
                    .post(
                        [this](auto& rq, auto& rs, auto sd)
                        {
                            handle_refunds_reverse(rq, rs, std::move(sd));
                        }
                    )
                    .summary("Hyperswitch refunds_reverse")
            )
        );
    }

    // original install() route #15
    void install_route_15(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_router(
            Router{router, ":id"}.add_route(
                Route{"manual-update"}
                    .put(
                        [this](auto& rq, auto& rs, auto sd)
                        {
                            handle_refunds_manual_update(rq, rs, std::move(sd));
                        }
                    )
                    .summary("Hyperswitch refunds_manual_update")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_4(router);
        install_route_5(router);
        install_route_15(router);
    }

private:
    ::app::context::AppContext& m_context;
    RefundsRuntime& m_runtime;
    RefundsInputSupport& m_input_support;
};

} // namespace app::handlers::refunds
