export module app_payment:refunds_handlers_v1_aggregate;

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

class RefundsV1AggregateHandlers
{
public:
    void bind_request_support(RefundsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    RefundsRequestSupport* m_request_support{nullptr};

    RefundsV1AggregateHandlers(
        ::app::context::AppContext& ctx,
        RefundsRuntime& runtime,
        RefundsInputSupport& input_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    /** @brief get_refunds_aggregates — routes/refunds.rs:794-824. */
    void handle_get_refunds_aggregates(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support->authorize(req, "Aggregates", spec_for("Aggregates"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);

        auto result = m_runtime.aggregates(m_request_support->context(req));
        result.handle(sink);
    }

    /** @brief get_refunds_aggregate_profile — routes/refunds.rs:851-887. */
    void handle_get_refunds_aggregate_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support
                    ->authorize(req, "AggregatesProfile", spec_for("AggregatesProfile"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);

        auto result = m_runtime.aggregates_profile(m_request_support->context(req));
        result.handle(sink);
    }

    // original install() route #12
    void install_route_12(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_route(
            Route{"aggregate"}
                .get(
                    [this](auto& rq, auto& rs, auto sd)
                    {
                        handle_get_refunds_aggregates(rq, rs, std::move(sd));
                    }
                )
                .summary("Hyperswitch get_refunds_aggregates")
        );
    }

    // original install() route #13
    void install_route_13(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_router(
            Router{router, "profile"}.add_route(
                Route{"aggregate"}
                    .get(
                        [this](auto& rq, auto& rs, auto sd)
                        {
                            handle_get_refunds_aggregate_profile(rq, rs, std::move(sd));
                        }
                    )
                    .summary("Hyperswitch get_refunds_aggregate_profile")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_12(router);
        install_route_13(router);
    }

private:
    ::app::context::AppContext& m_context;
    RefundsRuntime& m_runtime;
    RefundsInputSupport& m_input_support;
};

} // namespace app::handlers::refunds
