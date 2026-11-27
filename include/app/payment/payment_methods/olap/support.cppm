export module app_payment:payment_methods_handlers_olap_support;

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
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :payment_methods_input;
import :payment_methods_result;
import :payment_methods_runtime;

export namespace app::handlers::payment_methods {

class OlapSupportHandlers
{
public:
    void bind_request_support(PaymentMethodsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentMethodsRequestSupport* m_request_support{nullptr};

    OlapSupportHandlers(::app::context::AppContext& ctx, PaymentMethodsRuntime& runtime) noexcept :
        m_context{ctx},
        m_input_support{ctx},
        m_runtime{runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payment_methods"}
            .add_route(
                Route{"collect"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_initiate_pm_collect_link_flow(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch initiate_pm_collect_link_flow")
            )
            .add_router(
                Router{router, "collect"}.add_router(
                    Router{router, ":merchant_id"}.add_route(
                        Route{":collect_id"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    interfaces::io::ResponseSink sink{res, std::move(send)};

                                    handle_render_pm_collect_link(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch render_pm_collect_link")
                    )
                )
            )
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"details"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_payment_method_retrieve_olap_api(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payment_method_retrieve_olap_api")
                )
            );

        Router{router, ":pm_prefix/payment-methods"}.add_router(
            Router{router, ":id"}.add_route(
                Route{"details"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payment_method_retrieve_olap_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payment_method_retrieve_olap_api (v2 prefix)")
            )
        );
    }

    void handle_initiate_pm_collect_link_flow(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "initiate_pm_collect_link_flow",
            spec_for("initiate_pm_collect_link_flow")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<
            ::app::model::payment_methods::PaymentMethodCollectLinkRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant id"
            );
            error.send(sink);
            return;
        }
        if (!m_input_support.customer_exists(parsed->get_customer_id().get_value())) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Customer does not exist in our records"
            );
            error.send(sink);
            return;
        }
        const auto& dto = *parsed;
        auto result = m_runtime.initiate_pm_collect_link_flow(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_render_pm_collect_link(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "render_pm_collect_link",
            spec_for("render_pm_collect_link")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const std::string merchant_id = m_input_support.path_segment(req, 1);
        const std::string collect_id = m_input_support.path_segment(req);
        if (merchant_id.empty() || collect_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "collect link not found"
            );
            error.send(sink);
            return;
        }
        if (!m_input_support.collect_link_exists(collect_id)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "collect link not found"
            );
            error.send(sink);
            return;
        }
        const auto presented = ::app::http::RequestQuery{}(req).get("token");
        if (presented) {
            if (const auto secret_error =
                    m_input_support.check_client_secret(collect_id, *presented)) {
                const auto& error = *secret_error;
                error.send(sink);
                return;
            }
        }
        auto result = m_runtime.render_pm_collect_link(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_payment_method_retrieve_olap_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "payment_method_retrieve_olap_api",
            spec_for("payment_method_retrieve_olap_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const std::string payment_method_id = m_input_support.path_segment(req, 1);
        if (payment_method_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payment method id"
            );
            error.send(sink);
            return;
        }
        const auto record = m_input_support.find_payment_method(payment_method_id);
        if (!record) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment method does not exist in our records"
            );
            error.send(sink);
            return;
        }
        const auto& dto = *record;
        auto result =
            m_runtime.payment_method_retrieve_olap_api(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsRuntime& m_runtime;
};

} // namespace app::handlers::payment_methods
