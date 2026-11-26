export module app_payment:refunds_handlers_v2_create_retrieve;

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

class RefundsV2CreateRetrieveHandlers
{
public:
    RefundsV2CreateRetrieveHandlers(
        ::app::context::AppContext& ctx,
        RefundsRuntime& runtime,
        RefundsInputSupport& input_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    /** @brief refunds_create — routes/refunds.rs:90-123 (v1), :128-183 (v2). */
    void handle_refunds_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const bool v2 = m_input_support.is_v2(req);
        if (const auto denied = m_input_support.authorize_op(
                req,
                v2 ? AuthProfile::V2Create : AuthProfile::V1Create
            )) {
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

        const auto payment_id = m_input_support.json_string(body, "payment_id");
        RefundsCreateInput
            input{v2, payment_id, std::nullopt, std::nullopt, m_input_support.request_facts(req)};
        if (v2) {
            using Input = ::app::model::refunds::RefundsCreateRequest;
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                const auto error =
                    core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
                res.set_status(error.get_status());
                res.set_body(serde::Ser::serialize("application/json", error));
                send();
                return;
            }
            input.v2_payload = *parsed;
        } else {
            using Input = ::app::model::refunds::RefundRequest;
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                const auto error =
                    core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
                res.set_status(error.get_status());
                res.set_body(serde::Ser::serialize("application/json", error));
                send();
                return;
            }
            input.v1_payload = *parsed;
        }

        auto result = m_runtime.create(input);
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

    /** @brief refunds_retrieve — routes/refunds.rs:191-241 (v1), :245-292 (v2). */
    void handle_refunds_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_op(
                req,
                m_input_support.is_v2(req) ? AuthProfile::V2Retrieve : AuthProfile::V1Retrieve
            )) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto path_id = m_input_support.refund_id_from_path(req);
        if (!path_id.has_value()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing refund id in path");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        const bool force_sync = query.get("force_sync").value_or("false") == "true";
        const bool all_keys = query.get("all_keys_required").value_or("false") == "true";
        const bool include_raw =
            query.get("return_raw_connector_response").value_or("false") == "true";

        ::app::model::refunds::RefundsRetrieveRequest request{};
        request.set_refund_id(*path_id);
        request.set_force_sync(force_sync);
        request.set_all_keys_required(all_keys);
        RefundsRetrieveInput input{request, include_raw, m_input_support.request_facts(req)};

        auto result = m_runtime.retrieve(input);
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

    /** @brief refunds_retrieve_with_gateway_creds — routes/refunds.rs:296-350 (v2). */
    void handle_refunds_retrieve_with_gateway_creds(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_op(req, AuthProfile::V2Gateway)) {
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
        using Input = ::app::model::refunds::RefundsRetrievePayload;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto path_id = m_input_support.refund_id_from_path(req);
        if (!path_id.has_value()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing refund id in path");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        ::app::model::refunds::RefundsRetrieveRequest request{};
        request.set_refund_id(*path_id);
        request.set_force_sync(parsed->get_force_sync());
        request.set_all_keys_required(std::nullopt);
        if (parsed->get_merchant_connector_details()) {
            ::app::model::refunds::MerchantConnectorDetailsWrapValue wrap{};
            wrap.set_value(parsed->get_merchant_connector_details()->get_value());
            request.set_merchant_connector_details(wrap);
        }
        const bool include_raw = parsed->get_return_raw_connector_response().value_or(false);
        RefundsRetrieveGatewayInput input{request, include_raw, m_input_support.request_facts(req)};

        auto result = m_runtime.retrieve_gateway(input);
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

    // original install() route #16
    void install_route_16(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/refunds"}.add_route(
            Route{""}
                .post(
                    [this](auto& rq, auto& rs, auto sd)
                    {
                        handle_refunds_create(rq, rs, std::move(sd));
                    }
                )
                .summary("Hyperswitch refunds_create")
        );
    }

    // original install() route #17
    void install_route_17(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/refunds"}.add_route(
            Route{":id"}
                .get(
                    [this](auto& rq, auto& rs, auto sd)
                    {
                        handle_refunds_retrieve(rq, rs, std::move(sd));
                    }
                )
                .summary("Hyperswitch refunds_retrieve")
        );
    }

    // original install() route #18
    void install_route_18(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/refunds"}.add_route(
            Route{":id"}
                .post(
                    [this](auto& rq, auto& rs, auto sd)
                    {
                        handle_refunds_retrieve_with_gateway_creds(rq, rs, std::move(sd));
                    }
                )
                .summary("Hyperswitch refunds_retrieve_with_gateway_creds")
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_16(router);
        install_route_17(router);
        install_route_18(router);
    }

private:
    ::app::context::AppContext& m_context;
    RefundsRuntime& m_runtime;
    RefundsInputSupport& m_input_support;
};

} // namespace app::handlers::refunds
