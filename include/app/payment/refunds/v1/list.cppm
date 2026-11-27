export module app_payment:refunds_handlers_v1_list;

import std;
import core_behavior;
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
    void bind_request_support(RefundsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    RefundsRequestSupport* m_request_support{nullptr};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied = m_request_support->authorize(
                req,
                m_input_support.is_v2(req) ? "V2List" : "V1List",
                spec_for(m_input_support.is_v2(req) ? "V2List" : "V1List")
            )) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<::app::model::refunds::RefundListRequest>::pass(req, sink);
        if (!parsed) return;

        const bool profile_scoped = m_input_support.is_v2(req) ||
                                    req.get_path().find("/profile/list") != std::string_view::npos;
        const auto& dto = *parsed;

        auto result = m_runtime.list(m_request_support->context(req)));
        result.handle(sink);
    }

    /** @brief refunds_list_for_platform — routes/refunds.rs:576-605. */
    void handle_refunds_list_for_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support->authorize(req, "Platform", spec_for("Platform"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        ::app::model::refunds::PlatformRefundListRequest request{};
        m_input_support.apply_platform_query(::app::http::RequestQuery{}(req), request);
        const auto& dto = request;

        auto result = m_runtime.list_platform(dto, m_request_support->context(req));
        result.handle(sink);
    }

    /** @brief refunds_list_profile — routes/refunds.rs:647-682. */
    void handle_refunds_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support->authorize(req, "ProfileList", spec_for("ProfileList"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<::app::model::refunds::RefundListRequest>::pass(req, sink);
        if (!parsed) return;
        const auto& dto = *parsed;

        auto result = m_runtime.list_profile(dto, m_request_support->context(req));
        result.handle(sink);
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
