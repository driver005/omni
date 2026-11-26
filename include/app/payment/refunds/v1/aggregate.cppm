export module app_payment:refunds_handlers_v1_aggregate;

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

class RefundsV1AggregateHandlers
{
public:
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
        if (const auto denied = m_input_support.authorize_op(req, AuthProfile::Aggregates)) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        RefundsAggregatesInput input{
            query.get("start_time"),
            query.get("end_time"),
            m_input_support.request_facts(req)
        };

        auto result = m_runtime.aggregates(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    /** @brief get_refunds_aggregate_profile — routes/refunds.rs:851-887. */
    void handle_get_refunds_aggregate_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_op(req, AuthProfile::AggregatesProfile)) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        RefundsAggregatesProfileInput input{
            query.get("start_time"),
            query.get("end_time"),
            m_input_support.request_facts(req)
        };

        auto result = m_runtime.aggregates_profile(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
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
