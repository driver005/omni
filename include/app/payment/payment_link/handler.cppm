export module app_payment:payment_link_handler;

import std;
import core_behavior;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_store;
import app_http;
import app_metrics;
import app_crypto;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :payment_link_input;
import :payment_link_result;
import :payment_link_runtime;

export namespace app::handlers::payment_link {

class PaymentLinkRouteHandlers
{
public:
    // ─────────────────────────────────────────────────────────────────────────
    // Routes (routes/payment_link.rs)
    // ─────────────────────────────────────────────────────────────────────────

    void handle_payment_link_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support
                    .authorize(req, "payment_link_retrieve", spec_for("payment_link_retrieve"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        // Upstream takes Query<RetrievePaymentLinkRequest> (client_secret) plus a
        // path id. The request type is a typed model, not the response type.
        using Input = ::app::model::payments::RetrievePaymentLinkRequest;
        std::optional<Input> input{};
        const auto body = req.body_as_string();
        if (!body.empty()) {
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_JSON,
                    "invalid JSON"
                );
                error.send(sink);
                return;
            }
            input = *parsed;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        std::optional<std::string> client_secret = query.get("client_secret");
        if (!client_secret && input && input->get_client_secret()) {
            client_secret = *input->get_client_secret();
        }

        // check_sdk_auth_and_get_auth(): SDK key material plus an optional
        // client-secret assertion from the query payload (routes:22-42).
        const auto secret_claim = m_input_support.client_secret_claim(req);
        if (secret_claim && client_secret && *secret_claim != std::string_view{*client_secret}) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "client secret mismatch"
            );
            error.send(sink);
            return;
        }

        const std::string payment_link_id = m_input_support.path_link_id(req);
        if (payment_link_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payment link id"
            );
            error.send(sink);
            return;
        }

        const auto& dto = *input;
        auto result = m_runtime.retrieve(m_request_support.context(req)));
        result.handle(sink);
    }

    // initiate_payment_link (routes:66-98) -> initiate_payment_link_flow
    // (core:472-522). Upstream renders a PaymentLinkForm: either the payment
    // form data (js/css/sdk/meta/redirection) or the status page data.
    void handle_initiate_payment_link(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto ids = m_input_support.path_merchant_payment(req);
        const std::string merchant_id = ids.first;
        const std::string payment_id = ids.second;

        // MerchantIdAuth(merchant_id) from the path (routes:94).
        if (const auto denied =
                m_request_support
                    .authorize(req, "initiate_payment_link", spec_for("initiate_payment_link"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        if (merchant_id.empty() || payment_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "merchant id or payment id not found in path"
            );
            error.send(sink);
            return;
        }

        auto result = m_runtime.initiate(m_request_support.context(req));
        result.handle(sink);
    }

    // initiate_secure_payment_link (routes:100-133) ->
    // initiate_secure_payment_link_flow (core:362-470). Renders a GenericLinks
    // form (SecurePaymentLink) or the status page, after header validation.
    void handle_initiate_secure_payment_link(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto ids = m_input_support.path_merchant_payment(req);
        const std::string merchant_id = ids.first;
        const std::string payment_id = ids.second;

        // MerchantIdAuth(merchant_id) from the path (routes:129).
        if (const auto denied = m_request_support.authorize(
                req,
                "initiate_secure_payment_link",
                spec_for("initiate_secure_payment_link")
            )) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        if (merchant_id.empty() || payment_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "merchant id or payment id not found in path"
            );
            error.send(sink);
            return;
        }

        auto result = m_runtime.initiate_secure(m_request_support.context(req));
        result.handle(sink);
    }

    void handle_payments_link_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support
                    .authorize(req, "payments_link_list", spec_for("payments_link_list"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        using Input = ::app::model::payments::PaymentLinkListConstraints;
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(sink);
            return;
        }

        const auto auth = m_input_support.auth_merchant_and_profile(req);
        const auto& dto = *parsed;
        auto result = m_runtime.list(m_request_support.context(req)));
        result.handle(sink);
    }

    void handle_profile_payment_link_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied = m_request_support.authorize(
                req,
                "profile_payment_link_list",
                spec_for("profile_payment_link_list")
            )) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        using Input = ::app::model::payments::PaymentLinkListConstraints;
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(sink);
            return;
        }

        const auto auth = m_input_support.auth_merchant_and_profile(req);
        const auto& dto = *parsed;
        auto result = m_runtime.profile_list(dto, m_request_support.context(req));
        result.handle(sink);
    }

    // payment_link_status (routes:217-249) -> get_payment_link_status
    // (core:892-1073). Upstream always renders an IntentStatus page with
    // redirect=true plus the status-page js/css/redirection endpoint.
    void handle_payment_link_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto ids = m_input_support.path_merchant_payment(req);
        std::string merchant_id = ids.first;
        const std::string payment_id = ids.second;
        const auto header_merchant = m_input_support.merchant_id_from_header(req);
        if (merchant_id.empty()) {
            merchant_id = header_merchant;
        }

        // MerchantIdAuth(merchant_id) from the path (routes:245).
        if (const auto denied =
                m_request_support
                    .authorize(req, "payment_link_status", spec_for("payment_link_status"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        if (payment_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "payment id not found in path"
            );
            error.send(sink);
            return;
        }

        auto result = m_runtime.status(m_request_support.context(req));
        result.handle(sink);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payment_link"}
            .add_route(
                Route{"list"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_link_list(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_link_list")
            )
            .add_router(
                Router{router, "profile"}.add_route(
                    Route{"list"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_profile_payment_link_list(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch profile_payment_link_list")
                )
            )
            .add_route(
                Route{":payment_link_id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payment_link_retrieve(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payment_link_retrieve")
            )
            .add_router(
                Router{router, ":merchant_id"}.add_route(
                    Route{":payment_id"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_initiate_payment_link(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch initiate_payment_link")
                )
            )
            .add_router(
                Router{router, "s"}.add_router(
                    Router{router, ":merchant_id"}.add_route(
                        Route{":payment_id"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                    handle_initiate_secure_payment_link(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch initiate_secure_payment_link")
                    )
                )
            )
            .add_router(
                Router{router, "status"}.add_router(
                    Router{router, ":merchant_id"}.add_route(
                        Route{":payment_id"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                    handle_payment_link_status(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch payment_link_status")
                    )
                )
            );
    }

    explicit PaymentLinkRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
    }

private:
    PaymentLinkRuntime m_runtime;
    PaymentLinkInputSupport m_input_support;
    PaymentLinkPolicy m_policy;
    PaymentLinkRequestSupport m_request_support;
};

} // namespace app::handlers::payment_link
