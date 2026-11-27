export module app_payment:refunds_handlers_v2_create_retrieve;

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

class RefundsV2CreateRetrieveHandlers
{
public:
    void bind_request_support(RefundsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    RefundsRequestSupport* m_request_support{nullptr};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const bool v2 = m_input_support.is_v2(req);
        if (const auto denied = m_request_support->authorize(
                req,
                v2 ? "V2Create" : "V1Create",
                spec_for(v2 ? "V2Create" : "V1Create")
            )) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }

        const auto payment_id = m_input_support.json_string(body, "payment_id");
        if (v2) {
        auto parsed = core::behavior::Serializer<::app::model::refunds::RefundsCreateRequest>::pass(req, sink);
        if (!parsed) return;
        } else {
        auto parsed = core::behavior::Serializer<::app::model::refunds::RefundRequest>::pass(req, sink);
        if (!parsed) return;
        }

        auto result = m_runtime.create(dto, m_request_support->context(req));
        result.handle(sink);
    }

    /** @brief refunds_retrieve — routes/refunds.rs:191-241 (v1), :245-292 (v2). */
    void handle_refunds_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied = m_request_support->authorize(
                req,
                m_input_support.is_v2(req) ? "V2Retrieve" : "V1Retrieve",
                spec_for(m_input_support.is_v2(req) ? "V2Retrieve" : "V1Retrieve")
            )) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        const auto path_id = m_input_support.refund_id_from_path(req);
        if (!path_id.has_value()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing refund id in path");
            error.send(sink);
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
        const auto& dto = request;

        auto result = m_runtime.retrieve(m_request_support->context(req)));
        result.handle(sink);
    }

    /** @brief refunds_retrieve_with_gateway_creds — routes/refunds.rs:296-350 (v2). */
    void handle_refunds_retrieve_with_gateway_creds(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support->authorize(req, "V2Gateway", spec_for("V2Gateway"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<::app::model::refunds::RefundsRetrievePayload>::pass(req, sink);
        if (!parsed) return;

        const auto path_id = m_input_support.refund_id_from_path(req);
        if (!path_id.has_value()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing refund id in path");
            error.send(sink);
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
        const auto& dto = request;

        auto result = m_runtime.retrieve_gateway(dto, m_request_support->context(req));
        result.handle(sink);
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
