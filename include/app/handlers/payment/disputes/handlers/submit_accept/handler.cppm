export module app_payment:disputes_handlers_submit_accept;

import std;
import interfaces;
import core_router;
import core_serde;
import core_error;
import utils_openapi;
import app_model;
import app_context;
import app_auth;
import app_http;

import :disputes_input;
import :disputes_result;
import :disputes_runtime;

export namespace app::handlers::disputes {

class DisputesHandlersSubmitAccept
{
public:
    DisputesHandlersSubmitAccept(
        ::app::context::AppContext& context,
        DisputesRuntime& runtime,
        DisputesInputSupport& input_support
    ) noexcept :
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "disputes"}.add_router(
            Router{router, "accept"}.add_route(
                Route{":dispute_id"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_accept_dispute(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch accept_dispute")
            )
        );
    }

    void install_evidence(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "disputes"}.add_route(
            Route{"evidence"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_submit_dispute_evidence(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch submit_dispute_evidence")
        );
    }

    // ── routes/disputes.rs:441-479  accept_dispute ────────────────────────────
    void handle_accept_dispute(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied = m_input_support.authorize_dispute(req, ctx, kProfileWrite)) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto scope = m_input_support.scope_of(ctx, query, req);
        if (const auto invalid = m_input_support.scope_error(scope, false)) {
            const auto& error = *invalid;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        ::app::model::disputes::DisputeId dispute_id{};
        dispute_id.set_dispute_id(m_input_support.path_segment(req));
        if (dispute_id.get_dispute_id().empty()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        AcceptDisputeInput input{scope, dispute_id};
        auto result = m_runtime.accept_dispute(input);
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

    // ── routes/disputes.rs:496-532  submit_dispute_evidence ───────────────────
    void handle_submit_dispute_evidence(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied = m_input_support.authorize_dispute(req, ctx, kProfileWrite)) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto scope = m_input_support.scope_of(ctx, query, req);
        if (const auto invalid = m_input_support.scope_error(scope, false)) {
            const auto& error = *invalid;
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
        auto parsed = serde::Ser::deserialize<::app::model::disputes::SubmitEvidenceRequest>(
            "application/json",
            body
        );
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        SubmitDisputeEvidenceInput input{scope, *parsed};
        auto result = m_runtime.submit_dispute_evidence(input);
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

private:
    DisputesRuntime& m_runtime;            // SINGLE runtime, owned by the facade.
    DisputesInputSupport& m_input_support; // SINGLE stateless input helper, shared.
};

} // namespace app::handlers::disputes
