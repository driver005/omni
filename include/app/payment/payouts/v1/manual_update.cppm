export module app_payment:payouts_handlers_v1_manual_update;

import std;
import core_behavior;
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
import :context;
import :payouts_result;
import :payouts_runtime;

export namespace app::handlers::payouts {

class PayoutsManualUpdateHandlersV1
{
public:
    void bind_request_support(PayoutsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PayoutsRequestSupport* m_request_support{nullptr};

    explicit PayoutsManualUpdateHandlersV1(
        PayoutsRuntime& runtime,
        PayoutsInputSupport& input_support
    ) noexcept :
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_payouts_manual_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support
                    ->authorize(req, "payouts_manual_update", spec_for("payouts_manual_update"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<::app::model::payouts::PayoutsManualUpdateRequest>::pass(req, sink);
        if (!parsed) return;

        const auto path_id = m_input_support.payout_id_from_path(req);
        if (!path_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payout id in path"
            );
            error.send(sink);
            return;
        }

        const auto& dto = *parsed;
        auto result = m_runtime.manual_update(dto, ::app::payment::detail::make_call_context(req));
        result.handle(sink);
    }

    using Route = utils::openapi::ApiRoute;
    using Router = utils::openapi::ApiRouter;

    [[nodiscard]] Router install(Router chain, core::router::RouterContext<>& router)
    {
        return std::move(chain).add_router(
            Router{router, ":payout_id"}.add_route(
                Route{"manual-update"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payouts_manual_update(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payouts_manual_update")
            )
        );
    }

private:
    PayoutsRuntime& m_runtime;
    PayoutsInputSupport& m_input_support;
};

} // namespace app::handlers::payouts
