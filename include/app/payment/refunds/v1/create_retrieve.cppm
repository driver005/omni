export module app_payment:refunds_handlers_v1_create_retrieve;

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

class RefundsV1CreateRetrieveHandlers
{
public:
    void bind_request_support(RefundsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    RefundsRequestSupport* m_request_support{nullptr};

    RefundsV1CreateRetrieveHandlers(
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
        if (const auto denied = m_request_support->authorize(
                req,
                v2 ? "V2Create" : "V1Create",
                spec_for(v2 ? "V2Create" : "V1Create")
            )) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(res, send);
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
                error.send(res, send);
                return;
            }
            input.v2_payload = *parsed;
        } else {
            using Input = ::app::model::refunds::RefundRequest;
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                const auto error =
                    core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
                error.send(res, send);
                return;
            }
            input.v1_payload = *parsed;
        }

        auto result = m_runtime.create(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    /** @brief refunds_retrieve_with_body — routes/refunds.rs:358-392 (v1 sync). */
    void handle_refunds_retrieve_with_body(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support->authorize(req, "V1Sync", spec_for("V1Sync"))) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(res, send);
            return;
        }
        using Input = ::app::model::refunds::RefundsRetrieveRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            error.send(res, send);
            return;
        }

        const auto refund_id = !parsed->get_refund_id().empty()
                                   ? std::optional<std::string>{parsed->get_refund_id()}
                                   : ::app::http::RequestQuery{}(req).get("refund_id");
        if (!refund_id.has_value() || refund_id->empty()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing refund_id");
            error.send(res, send);
            return;
        }
        RefundsRetrieveBodyInput input{*parsed, m_input_support.request_facts(req)};

        auto result = m_runtime.retrieve_body(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    /** @brief refunds_retrieve — routes/refunds.rs:191-241 (v1), :245-292 (v2). */
    void handle_refunds_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support->authorize(
                req,
                m_input_support.is_v2(req) ? "V2Retrieve" : "V1Retrieve",
                spec_for(m_input_support.is_v2(req) ? "V2Retrieve" : "V1Retrieve")
            )) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto path_id = m_input_support.refund_id_from_path(req);
        if (!path_id.has_value()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing refund id in path");
            error.send(res, send);
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
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    // original install() route #1
    void install_route_1(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_route(
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

    // original install() route #2
    void install_route_2(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_route(
            Route{"sync"}
                .post(
                    [this](auto& rq, auto& rs, auto sd)
                    {
                        handle_refunds_retrieve_with_body(rq, rs, std::move(sd));
                    }
                )
                .summary("Hyperswitch refunds_retrieve_with_body")
        );
    }

    // original install() route #3
    void install_route_3(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "refunds"}.add_route(
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

    void install(core::router::RouterContext<>& router)
    {
        install_route_1(router);
        install_route_2(router);
        install_route_3(router);
    }

private:
    ::app::context::AppContext& m_context;
    RefundsRuntime& m_runtime;
    RefundsInputSupport& m_input_support;
};

} // namespace app::handlers::refunds
