export module app_payment:refunds_handlers_v1_list;

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

class RefundsV1ListHandlers
{
public:
    RefundsV1ListHandlers(
        ::app::context::AppContext& ctx,
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
        using Input = ::app::model::refunds::RefundListRequest;
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

    /** @brief refunds_list_for_platform — routes/refunds.rs:576-605. */
    void handle_refunds_list_for_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_op(req, AuthProfile::Platform)) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        ::app::model::refunds::PlatformRefundListRequest request{};
        m_input_support.apply_platform_query(::app::http::RequestQuery{}(req), request);
        RefundsListPlatformInput input{request, m_input_support.request_facts(req)};

        auto result = m_runtime.list_platform(input);
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

    /** @brief refunds_list_profile — routes/refunds.rs:647-682. */
    void handle_refunds_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_op(req, AuthProfile::ProfileList)) {
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
        using Input = ::app::model::refunds::RefundListRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        RefundsListProfileInput input{*parsed, m_input_support.request_facts(req)};

        auto result = m_runtime.list_profile(input);
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

    // original install() route #6
    void install_route_6(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_route(
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

    // original install() route #7
    void install_route_7(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_router(
            Router{router, "platform"}.add_route(
                Route{"list"}
                    .get(
                        [this](auto& rq, auto& rs, auto sd)
                        {
                            handle_refunds_list_for_platform(rq, rs, std::move(sd));
                        }
                    )
                    .summary("Hyperswitch refunds_list_for_platform")
            )
        );
    }

    // original install() route #9
    void install_route_9(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_router(
            Router{router, "profile"}.add_route(
                Route{"list"}
                    .post(
                        [this](auto& rq, auto& rs, auto sd)
                        {
                            handle_refunds_list_profile(rq, rs, std::move(sd));
                        }
                    )
                    .summary("Hyperswitch refunds_list_profile")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_6(router);
        install_route_7(router);
        install_route_9(router);
    }

private:
    ::app::context::AppContext& m_context;
    RefundsRuntime& m_runtime;
    RefundsInputSupport& m_input_support;
};

} // namespace app::handlers::refunds
