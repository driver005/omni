export module app_user:admin_handlers_v2_connectors;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_behavior;
import core_error;
import app_http;
import :admin_input;
import :admin_result;
import :admin_runtime;

export namespace app::handlers::admin {

class AdminHandlersV2Connectors
{
public:
    AdminHandlersV2Connectors(
        ::app::context::AppContext& context,
        AdminRuntime& runtime,
        AdminInputSupport& input_support,
        AdminRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_support{request_support}
    {
    }

    void handle_connector_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "connector_list", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        std::string merchant_id{::app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && m_input_support.path_has(req, "account")) {
            merchant_id = m_support.path(req, AdminPolicy::path_index(req, 2));
        }
        std::string profile_id_filter{
            ::app::http::RequestQuery{}(req).get("profile_id").value_or("")
        };
        const bool for_profile = m_input_support.path_has(req, "profiles");
        if (profile_id_filter.empty() && for_profile) {
            profile_id_filter = m_support.path(req, AdminPolicy::path_index(req, 2));
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{std::move(merchant_id)});
        ctx.claims.insert_or_assign("profile_id_filter", std::string{std::move(profile_id_filter)});
        ctx.claims.insert_or_assign("for_profile", for_profile ? "true" : "false");
        m_runtime.list_connectors(ctx).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // routes/admin.rs:740-775 (v2 connector_list for a profile).
        Router{router, "v2/profiles/:profile_id/connector-accounts"}.add_route(
            Route{""}
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
        );
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntime& m_runtime;
    AdminInputSupport& m_input_support;
    AdminRequestSupport& m_support;
};

} // namespace app::handlers::admin
