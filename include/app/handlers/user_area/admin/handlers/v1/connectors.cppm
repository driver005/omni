export module app_user:admin_handlers_v1_connectors;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_http;
import :admin_input;
import :admin_result;
import :admin_runtime;

export namespace app::handlers::admin {

class AdminHandlersV1Connectors
{
public:
    AdminHandlersV1Connectors(
        ::app::context::AppContext& context,
        AdminRuntime& runtime,
        AdminInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_connector_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto error = m_input_support.authorize(req, "connector_list")) {
            res.set_status(error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *error));
            send();
            return;
        }
        std::string merchant_id{::app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && m_input_support.path_has(req, "account")) {
            merchant_id = m_input_support.path_part(req, 2);
        }
        std::string profile_id_filter{
            ::app::http::RequestQuery{}(req).get("profile_id").value_or("")
        };
        const bool for_profile = m_input_support.path_has(req, "profiles");
        if (profile_id_filter.empty() && for_profile) {
            profile_id_filter = m_input_support.path_part(req, 2);
        }
        ConnectorListInput input{std::move(merchant_id), std::move(profile_id_filter), for_profile};
        auto result = m_runtime.list_connectors(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_items()));
        send();
    }

    void handle_connector_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto error = m_input_support.authorize(req, "connector_list_profile")) {
            res.set_status(error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *error));
            send();
            return;
        }
        std::string merchant_id{::app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && m_input_support.path_has(req, "account")) {
            merchant_id = m_input_support.path_part(req, 3);
        }
        ConnectorListProfileInput input{std::move(merchant_id)};
        auto result = m_runtime.list_connectors_profile(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_items()));
        send();
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // routes/app.rs:2233-2273 (MerchantConnectorAccount::server, v1).
        Router{router, "account"}.add_router(
            Router{router, ":merchant_id"}.add_route(
                Route{"connectors"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_connector_list(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch connector_list")
            )
        );
    }

    // Interleaved chain fragment — called from the version facade in original order.
    void install_profile(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // routes/app.rs:2792-2804 (ProfileNew::server, v1) — connector_list_profile.
        Router{router, "account/:account_id/profile/connectors"}.add_route(
            Route{""}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_connector_list_profile(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch connector_list_profile")
        );
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntime& m_runtime;
    AdminInputSupport& m_input_support;
};

} // namespace app::handlers::admin
