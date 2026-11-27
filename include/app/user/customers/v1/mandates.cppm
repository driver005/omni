export module app_user:customers_handlers_v1_mandates;

import std;
import interfaces;
import core_behavior;
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

import :customers_input;
import :customers_runtime;

export namespace app::handlers::customers {

class CustomersHandlersV1Mandates
{
public:
    CustomersHandlersV1Mandates(
        ::app::context::AppContext& context,
        CustomersRuntime& runtime,
        CustomersInputSupport& input_support,
        CustomersRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_support{request_support}
    {
    }

    void handle_get_customer_mandates(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_support.authorize(
                req,
                "get_customer_mandates",
                ::core::behavior::AuthSpec{
                    .permission = "MANDATE_READ",
                    .allow_connected = false,
                    .allow_platform = false
                }
            )) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        const std::string id = m_support.path(req, CustomersPolicy::path_index(req, 1));
        if (id.empty()) {
            const auto error =
                m_input_support.upstream_error("InvalidRequestData", "missing customer id");
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("id", std::string{id});
        ctx.claims.insert_or_assign(
            "merchant_id",
            std::string{m_input_support.request_merchant_id(req)}
        );
        m_runtime.get_customer_mandates(ctx).handle(sink);
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
    ::app::context::AppContext& m_context;
    CustomersRuntime& m_runtime;
    CustomersInputSupport& m_input_support;
    CustomersRequestSupport& m_support;
};

} // namespace app::handlers::customers
