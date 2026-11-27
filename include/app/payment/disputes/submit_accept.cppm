export module app_payment:disputes_handlers_submit_accept;

import std;
import core_behavior;
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
    void bind_request_support(DisputesRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    DisputesRequestSupport* m_request_support{nullptr};

    void bind_policy(DisputesPolicy& p) noexcept
    {
        m_policy = &p;
    }

    DisputesPolicy* m_policy{nullptr};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied =
                m_request_support
                    ->authorize(req, "ProfileDisputeWrite", kDisputeProfileWriteSpec)) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        const auto scope = m_request_support->scope(req);
        if (const auto invalid = m_request_support->scope_error(scope)) {
            const auto& error = *invalid;
            error.send(sink);
            return;
        }

        ::app::model::disputes::DisputeId dispute_id{};
        dispute_id.set_dispute_id(m_input_support.path_segment(req));
        if (dispute_id.get_dispute_id().empty()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id");
            error.send(sink);
            return;
        }
        const auto& dto = scope;
        auto result = m_runtime.accept_dispute(dto, m_request_support->context(req));
        result.handle(sink);
    }

    // ── routes/disputes.rs:496-532  submit_dispute_evidence ───────────────────
    void handle_submit_dispute_evidence(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied =
                m_request_support
                    ->authorize(req, "ProfileDisputeWrite", kDisputeProfileWriteSpec)) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        const auto scope = m_request_support->scope(req);
        if (const auto invalid = m_request_support->scope_error(scope)) {
            const auto& error = *invalid;
            error.send(sink);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }
        auto parsed = serde::Ser::deserialize<::app::model::disputes::SubmitEvidenceRequest>(
            "application/json",
            body
        );
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            error.send(sink);
            return;
        }
        const auto& dto = scope;
        auto result = m_runtime.submit_dispute_evidence(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    DisputesRuntime& m_runtime;            // SINGLE runtime, owned by the facade.
    DisputesInputSupport& m_input_support; // SINGLE stateless input helper, shared.
};

} // namespace app::handlers::disputes
