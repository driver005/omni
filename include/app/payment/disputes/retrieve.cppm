export module app_payment:disputes_handlers_retrieve;

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

class DisputesHandlersRetrieve
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

    DisputesHandlersRetrieve(
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
        Router{router, "disputes"}.add_route(
            Route{":dispute_id"}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_retrieve_dispute(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch retrieve_dispute")
        );
    }

    // ── routes/disputes.rs:33-69  retrieve_dispute ────────────────────────────
    void handle_retrieve_dispute(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied =
                m_request_support->authorize(req, "ProfileDisputeRead", kDisputeProfileReadSpec)) {
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

        ::app::model::disputes::DisputeRetrieveRequest request{};
        request.set_dispute_id(m_input_support.path_segment(req));
        if (request.get_dispute_id().empty()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id");
            error.send(sink);
            return;
        }
        if (const auto force = query.get("force_sync")) {
            request.set_force_sync(*force == "true" || *force == "1");
        }

        const auto& dto = scope;
        auto result = m_runtime.retrieve_dispute(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    DisputesRuntime& m_runtime;            // SINGLE runtime, owned by the facade.
    DisputesInputSupport& m_input_support; // SINGLE stateless input helper, shared.
};

} // namespace app::handlers::disputes
