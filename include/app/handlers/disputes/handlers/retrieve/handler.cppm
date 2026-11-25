export module app_handlers_disputes:handlers_retrieve;

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

import :input;
import :result;
import :runtime;

export namespace app::handlers::disputes {

class DisputesHandlersRetrieve
{
public:
    DisputesHandlersRetrieve(
        app::context::AppContext& context,
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
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = m_input_support.authorize_dispute(req, ctx, kProfileRead)) {
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

        app::model::disputes::DisputeRetrieveRequest request{};
        request.set_dispute_id(m_input_support.path_segment(req));
        if (request.get_dispute_id().empty()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        if (const auto force = query.get("force_sync")) {
            request.set_force_sync(*force == "true" || *force == "1");
        }

        RetrieveDisputeInput input{scope, request};
        auto result = m_runtime.retrieve_dispute(input);
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
