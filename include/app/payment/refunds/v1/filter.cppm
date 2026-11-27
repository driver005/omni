export module app_payment:refunds_handlers_v1_filter;

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

class RefundsV1FilterHandlers
{
public:
    void bind_request_support(RefundsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    RefundsRequestSupport* m_request_support{nullptr};

    RefundsV1FilterHandlers(
        ::app::context::AppContext& ctx,
        RefundsRuntime& runtime,
        RefundsInputSupport& input_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    /** @brief refunds_filter_list_for_platform — routes/refunds.rs:612-640. */
    void handle_refunds_filter_list_for_platform(
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

        auto result = m_runtime.filter_platform(m_request_support->context(req));
        result.handle(sink);
    }

    /** @brief refunds_filter_list — routes/refunds.rs:689-718 (TimeRange -> RefundListMetaData). */
    void handle_refunds_filter_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied = m_request_support->authorize(req, "Filter", spec_for("Filter"))) {
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
        // Upstream payload is common_utils::types::TimeRange { start_time, end_time }.
        const auto start = m_input_support.json_string(body, "start_time");
        const auto end = m_input_support.json_string(body, "end_time");
        if (body.find('{') == std::string::npos) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            error.send(sink);
            return;
        }

        auto result = m_runtime.filter_list(m_request_support->context(req));
        result.handle(sink);
    }

    /** @brief get_refunds_filters — routes/refunds.rs:725-750 (RefundListFilters). */
    void handle_get_refunds_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied = m_request_support->authorize(req, "Filters", spec_for("Filters"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        auto result = m_runtime.filters(m_request_support->context(req));
        result.handle(sink);
    }

    /** @brief get_refunds_filters_profile — routes/refunds.rs:757-790 (profile-scoped). */
    void handle_get_refunds_filters_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support->authorize(req, "FiltersProfile", spec_for("FiltersProfile"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        auto result = m_runtime.filters_profile(m_request_support->context(req));
        result.handle(sink);
    }

    // original install() route #8
    void install_route_8(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_router(
            Router{router, "platform"}.add_route(
                Route{"filter"}
                    .get(
                        [this](auto& rq, auto& rs, auto sd)
                        {
                            handle_refunds_filter_list_for_platform(rq, rs, std::move(sd));
                        }
                    )
                    .summary("Hyperswitch refunds_filter_list_for_platform")
            )
        );
    }

    // original install() route #10
    void install_route_10(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_route(
            Route{"filter"}
                .post(
                    [this](auto& rq, auto& rs, auto sd)
                    {
                        handle_refunds_filter_list(rq, rs, std::move(sd));
                    }
                )
                .summary("Hyperswitch refunds_filter_list")
        );
    }

    // original install() route #11
    void install_route_11(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_router(
            Router{router, "v2"}.add_route(
                Route{"filter"}
                    .get(
                        [this](auto& rq, auto& rs, auto sd)
                        {
                            handle_get_refunds_filters(rq, rs, std::move(sd));
                        }
                    )
                    .summary("Hyperswitch get_refunds_filters")
            )
        );
    }

    // original install() route #14
    void install_route_14(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_router(
            Router{router, "v2"}.add_router(
                Router{router, "profile"}.add_route(
                    Route{"filter"}
                        .get(
                            [this](auto& rq, auto& rs, auto sd)
                            {
                                handle_get_refunds_filters_profile(rq, rs, std::move(sd));
                            }
                        )
                        .summary("Hyperswitch get_refunds_filters_profile")
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_8(router);
        install_route_10(router);
        install_route_11(router);
        install_route_14(router);
    }

private:
    ::app::context::AppContext& m_context;
    RefundsRuntime& m_runtime;
    RefundsInputSupport& m_input_support;
};

} // namespace app::handlers::refunds
