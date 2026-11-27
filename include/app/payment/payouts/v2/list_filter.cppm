export module app_payment:payouts_handlers_v2_list_filter;

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

class PayoutsListFilterHandlersV2
{
public:
    void bind_request_support(PayoutsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PayoutsRequestSupport* m_request_support{nullptr};

    explicit PayoutsListFilterHandlersV2(
        PayoutsRuntime& runtime,
        PayoutsInputSupport& input_support
    ) noexcept :
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_get_payout_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_request_support
                    ->authorize(req, "get_payout_filters", spec_for("get_payout_filters"))) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }

        PayoutsGetFiltersInput input{m_input_support.merchant_id(req)};
        auto result = m_runtime.get_payout_filters(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_get_payout_filters_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support->authorize(
                req,
                "get_payout_filters_profile",
                spec_for("get_payout_filters_profile")
            )) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }

        PayoutsGetFiltersProfileInput input{
            m_input_support.merchant_id(req),
            m_input_support.profile_id(req, std::nullopt)
        };
        auto result = m_runtime.get_payout_filters_profile(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    using Route = utils::openapi::ApiRoute;
    using Router = utils::openapi::ApiRouter;

    [[nodiscard]] Router install(Router chain, core::router::RouterContext<>& router)
    {
        return std::move(chain)
            .add_router(
                Router{router, "v2"}.add_route(
                    Route{"filter"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_get_payout_filters(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_payout_filters")
                )
            )
            .add_router(
                Router{router, "v2"}.add_router(
                    Router{router, "profile"}.add_route(
                        Route{"filter"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_get_payout_filters_profile(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch get_payout_filters_profile")
                    )
                )
            );
    }

private:
    PayoutsRuntime& m_runtime;
    PayoutsInputSupport& m_input_support;
};

} // namespace app::handlers::payouts
