export module app_handlers_customers:handlers_v1_mandates;

import std;
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

import :input;
import :result;
import :runtime;

export namespace app::handlers::customers {

class CustomersHandlersV1Mandates
{
public:
    CustomersHandlersV1Mandates(
        app::context::AppContext& context,
        CustomersRuntime& runtime,
        CustomersInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_get_customer_mandates(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize(
                req,
                ::app::auth::AppPermission::MANDATE_READ,
                m_input_support.is_jwt_auth(req),
                false,
                false,
                false,
                true,
                false,
                false
            )) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string id = m_input_support.path_segment(req, 1);
        if (id.empty()) {
            const auto error =
                m_input_support.upstream_error("InvalidRequestData", "missing customer id");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        CustomersMandatesRouteInput input{id, m_input_support.request_merchant_id(req)};
        auto result = m_runtime.get_customer_mandates(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    /** @brief Owns the `:customer_id/mandates` sub-router (v1-only route). */
    [[nodiscard]] utils::openapi::ApiRouter make_router(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        return Router{router, ":customer_id"}.add_route(
            Route{"mandates"}
                .get(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_get_customer_mandates(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch get_customer_mandates")
        );
    }

private:
    app::context::AppContext& m_context;
    CustomersRuntime& m_runtime;
    CustomersInputSupport& m_input_support;
};

} // namespace app::handlers::customers
