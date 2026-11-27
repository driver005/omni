export module app_payment:card_issuer_handler;

import std;
import core_behavior;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_dto;
import app_context;
import core_error;

import :card_issuer_input;
import :card_issuer_result;
import :card_issuer_runtime;

export namespace app::handlers::card_issuer {

/**
 * Card issuer route handlers.
 *
 * The four public `handle_*` methods are local ports of the upstream route
 * wrappers in `docs/hyperswitch/crates/router/src/routes/card_issuer.rs`
 * (`add_card_issuer`, `update_card_issuer`, `delete_card_issuer`,
 * `list_card_issuers`). Each one performs the route-layer concerns that upstream
 * delegates to `api::server_wrap`: auth, JSON body extraction, error dispatch
 * and response emission.
 *
 * Business logic lives in the `runtime/` partitions; each sub-runtime is a
 * direct port of the matching upstream core function in
 * `docs/hyperswitch/crates/router/src/core/card_issuer.rs`.
 */
class CardIssuerRouteHandlers
{
public:
    /** @brief Port of routes/card_issuer.rs:12-30 `add_card_issuer` (AdminApiAuth). */
    void handle_add_card_issuer(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support.authorize(req, "add_card_issuer", spec_for("add_card_issuer"))) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = m_input_support.make_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }

        // Upstream `web::Json<api_types::CardIssuerRequest>` extraction.
        auto parsed =
            core::behavior::Serializer<::app::dto::card_issuer::CardIssuerRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }

        const auto& dto = *parsed;
        auto result = m_runtime.add(dto, m_request_support.context(req));
        result.handle(sink);
    }

    /** @brief Port of routes/card_issuer.rs:32-52 `update_card_issuer` (AdminApiAuth). */
    void handle_update_card_issuer(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support
                    .authorize(req, "update_card_issuer", spec_for("update_card_issuer"))) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        // Upstream `web::Path<id_type::CardIssuerId>` extraction (path id only).
        const std::string id = m_request_support.path(req, 1);

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = m_input_support.make_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }

        // Upstream `web::Json<api_types::CardIssuerUpdateRequest>` extraction.
        auto parsed =
            core::behavior::Serializer<::app::dto::card_issuer::CardIssuerUpdateRequest>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }

        const auto& dto = id;
        auto result = m_runtime.update(dto, m_request_support.context(req));
        result.handle(sink);
    }

    /** @brief Port of routes/card_issuer.rs:54-74 `delete_card_issuer` (AdminApiAuth). */
    void handle_delete_card_issuer(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support
                    .authorize(req, "delete_card_issuer", spec_for("delete_card_issuer"))) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        // Upstream builds `CardIssuerDeleteRequest { id: path.into_inner() }` —
        // the id comes from the path only (no body fallback upstream).
        const std::string id = m_request_support.path(req, 1);

        auto result = m_runtime.remove(m_request_support.context(req));
        result.handle(sink);
    }

    /**
     * @brief Port of routes/card_issuer.rs:76-100 `list_card_issuers`
     * (`auth_type(HeaderAuth(ApiKeyAuth), JWTAuth(MerchantAccountRead), headers)`).
     */
    void handle_list_card_issuers(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        // The `list_jwt_fallback` flag models upstream `auth_type`: the
        // ApiKeyAuth branch (default) OR the JWTAuth(MerchantAccountRead) branch,
        // both with allow_connected:false and allow_platform:false.
        if (const auto auth_error =
                m_request_support
                    .authorize(req, "list_card_issuers", spec_for("list_card_issuers"))) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        auto result = m_runtime.list(m_request_support.context(req));
        result.handle(sink);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Upstream app.rs:2093-2105: scope "/card_issuers" with resource "" (POST/GET)
        // and resource "/{id}" (PUT/DELETE). `delt()` is the router's DELETE binding
        // (core/router/builder.cppm:540-548), the local equivalent of `web::delete()`.
        Router{router, "card_issuers"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_add_card_issuer(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch add_card_issuer")
            )
            .add_route(
                Route{""}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_list_card_issuers(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_card_issuers")
            )
            .add_route(
                Route{":id"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_update_card_issuer(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch update_card_issuer")
            )
            .add_route(
                Route{":id"}
                    .delt(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_delete_card_issuer(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch delete_card_issuer")
            );
    }

    explicit CardIssuerRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
    }

private:
    CardIssuerRuntime m_runtime;
    CardIssuerInputSupport m_input_support;
    CardIssuerPolicy m_policy;
    CardIssuerRequestSupport m_request_support;
};

} // namespace app::handlers::card_issuer
