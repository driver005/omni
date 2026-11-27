export module app_routing:gsm_handlers_v1;

import std;
import interfaces;
import core_router;
import core_behavior;
import core_serde;
import core_error;
import utils_openapi;
import app_context;
import app_dto;

import :gsm_input;
import :gsm_result;
import :gsm_runtime;

export namespace app::handlers::gsm {

/**
 * v1 surface: upstream `web::scope("/gsm")` with POST "", "get", "update",
 * "delete" (routes/app.rs:2813-2835).
 *
 * Each handle_* is a thin adapter: authorize -> parse -> Input -> runtime ->
 * response. Runtime and input support are owned once by the facade and shared
 * by reference. v1 and v2 each carry their own copy of these adapters because
 * upstream registers the same four operations under both scopes.
 */
class GsmHandlersV1
{
public:
    GsmHandlersV1(
        ::app::context::AppContext& context,
        GsmRuntime& runtime,
        GsmInputSupport& input_support,
        GsmRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support}
    {
    }

    void handle_create_gsm_rule(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_request_support.authorize(req, "create_gsm_rule", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto parsed =
            core::behavior::Serializer<::app::dto::gsm::GsmCreateRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        m_runtime.create(*parsed, m_request_support.context(req)).handle(sink);
    }

    void handle_get_gsm_rule(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_request_support.authorize(req, "get_gsm_rule", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto parsed =
            core::behavior::Serializer<::app::dto::gsm::GsmRetrieveRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        m_runtime.get(*parsed, m_request_support.context(req)).handle(sink);
    }

    void handle_update_gsm_rule(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_request_support.authorize(req, "update_gsm_rule", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto parsed =
            core::behavior::Serializer<::app::dto::gsm::GsmUpdateRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        m_runtime.update(*parsed, m_request_support.context(req)).handle(sink);
    }

    void handle_delete_gsm_rule(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_request_support.authorize(req, "delete_gsm_rule", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto parsed =
            core::behavior::Serializer<::app::dto::gsm::GsmDeleteRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        m_runtime.remove(*parsed, m_request_support.context(req)).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "gsm"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_create_gsm_rule(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch create_gsm_rule")
            )
            .add_route(
                Route{"get"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_gsm_rule(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_gsm_rule")
            )
            .add_route(
                Route{"update"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_update_gsm_rule(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch update_gsm_rule")
            )
            .add_route(
                Route{"delete"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_delete_gsm_rule(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch delete_gsm_rule")
            );
    }

private:
    ::app::context::AppContext& m_context;
    GsmRuntime& m_runtime;
    GsmInputSupport& m_input_support;
    GsmRequestSupport& m_request_support;
};

} // namespace app::handlers::gsm
