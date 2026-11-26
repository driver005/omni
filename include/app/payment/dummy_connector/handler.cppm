export module app_payment:dummy_connector_handler;

import std;
import interfaces;
import core_router;
import core_serde;
import core_error;
import utils_openapi;
import app_context;
import app_http;

import :dummy_connector_input;
import :dummy_connector_result;
import :dummy_connector_runtime;

export namespace app::handlers::dummy_connector {

class DummyConnectorRouteHandlers
{
public:
    // -----------------------------------------------------------------------
    // routes/dummy_connector.rs:18-36 -> core.rs:59-85 (payment_authorize).
    // -----------------------------------------------------------------------
    void handle_dummy_connector_authorize_payment(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "dummy_connector_authorize_payment")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        AuthorizePaymentInput input{m_input_support.last_segment(req)};
        auto result = m_runtime.authorize_payment(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.add_content_type("text/html");
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_html()));
        send();
    }

    // -----------------------------------------------------------------------
    // routes/dummy_connector.rs:40-62 -> core.rs:88-153 (payment_complete).
    // -----------------------------------------------------------------------
    void handle_dummy_connector_complete_payment(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "dummy_connector_complete_payment")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        m_input_support.apply_mock_latency("payment_duration", "payment_tolerance");

        const auto attempt_id = m_input_support.last_segment(req);
        const auto confirm_value = ::app::http::RequestQuery{}(req).get("confirm");
        if (!confirm_value || (*confirm_value != "true" && *confirm_value != "false")) {
            // web::Query<DummyConnectorPaymentCompleteBody> requires a bool.
            const auto error = m_input_support.dc_missing_field("confirm");
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const bool confirm = *confirm_value == "true";

        CompletePaymentInput input{attempt_id, confirm};
        auto result = m_runtime.complete_payment(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.add_content_type("application/json");
        res.set_status(interfaces::io::types::Status::FOUND);
        res.set_header(interfaces::io::types::Token::LOCATION, result->get_location());
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    // -----------------------------------------------------------------------
    // routes/dummy_connector.rs:66-83 -> core.rs:13-42 (payment).
    // -----------------------------------------------------------------------
    void handle_dummy_connector_payment(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "dummy_connector_payment")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        if (m_input_support.require_local_host(req)) {
            res.set_status(interfaces::io::types::Status::NOT_FOUND);
            res.set_body(std::vector<std::byte>{});
            send();
            return;
        }
        m_input_support.apply_mock_latency("payment_duration", "payment_tolerance");

        auto parsed = m_input_support.parse_payment_request(req.body_as_string());
        if (!parsed) {
            const auto& error = parsed.error();
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        PaymentInput input{*parsed};
        auto result = m_runtime.payment(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.add_content_type("application/json");
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    // -----------------------------------------------------------------------
    // routes/dummy_connector.rs:87-105 -> core.rs:44-56 (payment_data).
    // -----------------------------------------------------------------------
    void handle_dummy_connector_payment_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "dummy_connector_payment_data")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        if (m_input_support.require_local_host(req)) {
            res.set_status(interfaces::io::types::Status::NOT_FOUND);
            res.set_body(std::vector<std::byte>{});
            send();
            return;
        }
        m_input_support.apply_mock_latency(
            "payment_retrieve_duration",
            "payment_retrieve_tolerance"
        );

        PaymentDataInput input{m_input_support.last_segment(req)};
        auto result = m_runtime.payment_data(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.add_content_type("application/json");
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    // -----------------------------------------------------------------------
    // routes/dummy_connector.rs:109-128 -> core.rs:156-207 (refund_payment).
    // -----------------------------------------------------------------------
    void handle_dummy_connector_refund(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "dummy_connector_refund")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        if (m_input_support.require_local_host(req)) {
            res.set_status(interfaces::io::types::Status::NOT_FOUND);
            res.set_body(std::vector<std::byte>{});
            send();
            return;
        }
        m_input_support.apply_mock_latency("refund_duration", "refund_tolerance");

        // `/:payment_id/refund`: payment_id is the segment before `refund`.
        const auto segments = ::app::http::RequestPath{}(req);
        if (segments.count() < 2) {
            const auto error = m_input_support.dc_missing_field("payment_id");
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string payment_id{segments.at(segments.count() - 2).value_or("")};
        if (payment_id.empty()) {
            const auto error = m_input_support.dc_missing_field("payment_id");
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        auto parsed = m_input_support.parse_refund_request(req.body_as_string());
        if (!parsed) {
            const auto& error = parsed.error();
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        RefundInput input{payment_id, parsed->amount};
        auto result = m_runtime.refund(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.add_content_type("application/json");
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // -----------------------------------------------------------------------
    // routes/dummy_connector.rs:132-150 -> core.rs:210-234 (refund_data).
    // -----------------------------------------------------------------------
    void handle_dummy_connector_refund_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "dummy_connector_refund_data")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        if (m_input_support.require_local_host(req)) {
            res.set_status(interfaces::io::types::Status::NOT_FOUND);
            res.set_body(std::vector<std::byte>{});
            send();
            return;
        }
        m_input_support.apply_mock_latency("refund_retrieve_duration", "refund_retrieve_tolerance");

        RefundDataInput input{m_input_support.last_segment(req)};
        auto result = m_runtime.refund_data(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.add_content_type("application/json");
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.add_content_type("application/json");
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "dummy-connector"}
            .add_router(
                Router{router, "authorize"}.add_route(
                    Route{":attempt_id"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_dummy_connector_authorize_payment(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch dummy_connector_authorize_payment")
                )
            )
            .add_router(
                Router{router, "complete"}.add_route(
                    Route{":attempt_id"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_dummy_connector_complete_payment(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch dummy_connector_complete_payment")
                )
            )
            .add_route(
                Route{"payment"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_dummy_connector_payment(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch dummy_connector_payment")
            )
            .add_router(
                Router{router, "payments"}.add_route(
                    Route{":payment_id"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_dummy_connector_payment_data(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch dummy_connector_payment_data")
                )
            )
            .add_router(
                Router{router, ":payment_id"}.add_route(
                    Route{"refund"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_dummy_connector_refund(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch dummy_connector_refund")
                )
            )
            .add_router(
                Router{router, "refunds"}.add_route(
                    Route{":refund_id"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_dummy_connector_refund_data(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch dummy_connector_refund_data")
                )
            );
    }

    explicit DummyConnectorRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    DummyConnectorRuntime m_runtime;
    DummyConnectorInputSupport m_input_support;
};

} // namespace app::handlers::dummy_connector
