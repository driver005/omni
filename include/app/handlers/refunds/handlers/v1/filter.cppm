export module app_handlers_refunds:handlers_v1_filter;

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

class RefundsV1FilterHandlers
{
public:
    RefundsV1FilterHandlers(
        app::context::AppContext& ctx,
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
        if (const auto denied = m_input_support.authorize_op(req, AuthProfile::Platform)) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        RefundsFilterPlatformInput input{m_input_support.request_facts(req)};

        auto result = m_runtime.filter_platform(input);
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

    /** @brief refunds_filter_list — routes/refunds.rs:689-718 (TimeRange -> RefundListMetaData). */
    void handle_refunds_filter_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_op(req, AuthProfile::Filter)) {
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
        // Upstream payload is common_utils::types::TimeRange { start_time, end_time }.
        const auto start = m_input_support.json_string(body, "start_time");
        const auto end = m_input_support.json_string(body, "end_time");
        if (body.find('{') == std::string::npos) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        RefundsFilterListInput input{start, end, m_input_support.request_facts(req)};

        auto result = m_runtime.filter_list(input);
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

    /** @brief get_refunds_filters — routes/refunds.rs:725-750 (RefundListFilters). */
    void handle_get_refunds_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_op(req, AuthProfile::Filters)) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        RefundsFiltersInput input{m_input_support.request_facts(req)};

        auto result = m_runtime.filters(input);
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

    /** @brief get_refunds_filters_profile — routes/refunds.rs:757-790 (profile-scoped). */
    void handle_get_refunds_filters_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_op(req, AuthProfile::FiltersProfile)) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        RefundsFiltersProfileInput input{m_input_support.request_facts(req)};

        auto result = m_runtime.filters_profile(input);
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
    app::context::AppContext& m_context;
    RefundsRuntime& m_runtime;
    RefundsInputSupport& m_input_support;
};

} // namespace app::handlers::refunds
