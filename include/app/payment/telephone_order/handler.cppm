export module app_payment:telephone_order_handler;

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
import app_locking;
import app_events;
import app_workflow;
import app_keystore;

import :telephone_order_input;
import :context;
import :telephone_order_result;
import :telephone_order_runtime;

export namespace app::handlers::telephone_order {

class TelephoneOrderRouteHandlers
{
public:
    /**
     * @brief routes/telephone_order.rs:19-36 — telephone_order_charge.
     * Routes to the Twilio adapter; an unknown provider is InvalidRequestUrl.
     */
    void handle_telephone_order_charge(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        // 1. request -> Input.
        const ChargePath path = m_input_support.charge_path(req);
        if (path.merchant_id.empty() || path.provider != "twilio") {
            m_context->metrics().counter("telephone_order_charge_invalid_request_url").increment();
            const auto error = invalid_request_url_error();
            error.send(sink);
            return;
        }

        const std::string body = req.body_as_string();
        auto parsed = serde::Ser::deserialize<TwilioGenericPayRequest>("application/json", body);
        // An unknown TwilioPayMethod is a deserialization failure upstream (enum variant).
        if (parsed && parsed->get_method() != "charge" && parsed->get_method() != "tokenize") {
            parsed = std::unexpected{std::string{"Unknown TwilioPayMethod"}};
        }
        if (!parsed) {
            m_context->metrics().counter("telephone_order_charge_invalid_data_format").increment();
            const auto error = invalid_data_format_error();
            error.send(sink);
            return;
        }

        const auto& dto = *parsed;

        // 2. Input -> Result.
        auto result = m_runtime.telephone_order_charge(dto, ::app::payment::detail::make_call_context(req));
        result.handle(sink);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "telephone_order"}.add_router(
            Router{router, ":merchant_id"}.add_route(
                Route{":provider"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_telephone_order_charge(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch telephone_order_charge")
            )
        );
    }

    explicit TelephoneOrderRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{&context},
        m_runtime{context},
        m_input_support{context},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
    }

private:
    /** IR_02 invalid_request: real message in `message`, upstream code in `error`. */
    [[nodiscard]] static core::error::AppError invalid_request_url_error()
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::NOT_FOUND,
            "Unrecognized request URL"
        );
        error.set_kind_name("IR_02");
        return error;
    }

    /** IR_05 invalid_request: real message in `message`, upstream code in `error`. */
    [[nodiscard]] static core::error::AppError invalid_data_format_error()
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::VALIDATION_FAILED,
            "request body contains invalid data. Expected format is a telephone order charge "
            "request for this provider"
        );
        error.set_kind_name("IR_05");
        return error;
    }

    ::app::context::AppContext* m_context{nullptr};
    TelephoneOrderRuntime m_runtime;
    TelephoneOrderInputSupport m_input_support;
    TelephoneOrderPolicy m_policy;
    TelephoneOrderRequestSupport m_request_support;
};

} // namespace app::handlers::telephone_order
