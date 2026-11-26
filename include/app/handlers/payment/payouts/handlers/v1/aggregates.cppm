export module app_payment:payouts_handlers_v1_aggregates;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;
import app_locking;

import :payouts_input;
import :payouts_result;
import :payouts_runtime;

export namespace app::handlers::payouts {

class PayoutsAggregatesHandlersV1
{
public:
    explicit PayoutsAggregatesHandlersV1(
        PayoutsRuntime& runtime,
        PayoutsInputSupport& input_support
    ) noexcept :
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_get_payouts_aggregates(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_input_support.authorize_operation(req, "get_payouts_aggregates")) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // Upstream input is `web::Query<TimeRange>` (`payouts.rs:708`); no body is read.
        PayoutsAggregatesInput input{
            m_input_support.time_range_from_query(m_input_support.query(req))
        };
        auto result = m_runtime.get_payouts_aggregates(input);
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

    void handle_get_payouts_aggregates_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_input_support.authorize_operation(req, "get_payouts_aggregates_profile")) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        PayoutsAggregatesProfileInput input{
            m_input_support.time_range_from_query(m_input_support.query(req)),
            m_input_support.profile_id(req, std::nullopt)
        };
        auto result = m_runtime.get_payouts_aggregates_profile(input);
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

    using Route = utils::openapi::ApiRoute;
    using Router = utils::openapi::ApiRouter;

    [[nodiscard]] Router install(Router chain, core::router::RouterContext<>& router)
    {
        return std::move(chain)
            .add_route(
                Route{"aggregate"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_payouts_aggregates(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_payouts_aggregates")
            )
            .add_router(
                Router{router, "profile"}.add_route(
                    Route{"aggregate"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_get_payouts_aggregates_profile(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_payouts_aggregates_profile")
                )
            );
    }

private:
    PayoutsRuntime& m_runtime;
    PayoutsInputSupport& m_input_support;
};

} // namespace app::handlers::payouts
