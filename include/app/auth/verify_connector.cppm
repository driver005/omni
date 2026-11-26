export module app_auth:verify_connector_handler;

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
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_connector;

import :verify_connector_input;
import :verify_connector_result;
import :verify_connector_runtime;

export namespace app::handlers::verify_connector {

class VerifyConnectorRouteHandlers
{
    using Error = core::error::AppError;
    using Kind = core::error::AppErrorKind;

public:
    void handle_payment_connector_verify(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // JWTAuth { permission: MerchantConnectorWrite, allow_connected: true,
        // allow_platform: false }. The auth decision and merchant identity are resolved by
        // the input support; auth failures are written here with no runtime call.
        const auto authorized = m_input_support.authorize(req);
        if (authorized.error) {
            const auto& error = *authorized.error;
            error.send(res, send);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(res, send);
            return;
        }

        // Typed upstream request (api_models/src/verify_connector.rs:5-9): `connector_name` is a
        // plain snake_case string validated into the Connector enum, and
        // `connector_account_details` is the internally-tagged ConnectorAuthType object.
        using Dto = ::app::dto::verify_connector::VerifyConnectorRequest;
        auto parsed = serde::Ser::deserialize<Dto>("application/json", body);
        if (!parsed) {
            const auto error =
                m_input_support.invalid_request_data_error("invalid VerifyConnectorRequest");
            error.send(res, send);
            return;
        }

        const auto connector =
            ::app::dto::verify_connector::connector_from_wire(parsed->get_connector_name());
        if (!connector) {
            // Upstream serde rejects an unknown Connector enum value during web::Json extract.
            const auto error = m_input_support.incorrect_connector_name_given_error();
            error.send(res, send);
            return;
        }
        const std::string connector_name{
            ::app::dto::verify_connector::connector_to_wire(*connector)
        };

        PaymentConnectorVerifyInput
            input{*parsed, *connector, connector_name, authorized.merchant_id};
        auto result = m_runtime.payment_connector_verify(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        // StatusOk with no payload. The old send_json(res, send, {}, true) wrote an empty raw
        // body; the typed result now carries that as an empty serde::Value document and the
        // handler writes it through Ser::serialize.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    // routes/verify_connector.rs:36 uses api_locking::LockAction::NotApplicable: no lock is
    // acquired on this route. The local LockRegistry is therefore deliberately unused here.
    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "account"}.add_router(
            Router{router, "connectors"}.add_route(
                Route{"verify"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payment_connector_verify(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payment_connector_verify")
            )
        );
    }

    explicit VerifyConnectorRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    VerifyConnectorRuntime m_runtime;
    VerifyConnectorInputSupport m_input_support;
};

} // namespace app::handlers::verify_connector
