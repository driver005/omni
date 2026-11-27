export module app_user:hierarchical_resources_handler;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
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

import :hierarchical_resources_input;
import :hierarchical_resources_result;
import :hierarchical_resources_runtime;

export namespace app::handlers::hierarchical_resources {

class HierarchicalResourcesRouteHandlers
{
public:
    explicit HierarchicalResourcesRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context}
    {
    }

    // ── Route handlers: parse body, resolve mandatory X-Profile-Id, authorize
    //    (upstream server_wrap), then dispatch to the domain runtime. ──────

    void handle_generate_hierarchical_resource(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream: web::Json<GenerateHierarchicalResourceRequest> is decoded by the
        // framework before the handler (routes:77); a bad body is a 400.
        auto parsed = core::behavior::Serializer<::app::dto::hierarchical_resources::GenerateHierarchicalResourceRequest>::pass(req, sink);
        if (!parsed) return;
        const auto resource_type = parsed->get_resource_type().get_value();
        if (!m_input_support.valid_resource_type(resource_type)) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid resource type"
            );
            error.send(sink);
            return;
        }

        // routes:81-84 — mandatory_profile_id_from_header runs before server_wrap.
        const auto profile_id = m_input_support.mandatory_profile_id_from_header(req);
        if (!profile_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing X-Profile-Id header"
            );
            error.send(sink);
            return;
        }
        const auto auth_error =
            m_support.authorize(req, "generate_hierarchical_resource", {});
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = m_input_support.resolve_merchant_id(req, auth);
        // routes:93 — resolve_profile_from_header: scoped fetch is the auth check.
        if (!m_input_support.resolve_profile_from_header(*profile_id, merchant_id).has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "profile not found for merchant"
            );
            error.send(sink);
            return;
        }
        const auto organization_id = m_input_support.resolve_scope_id(req, auth, merchant_id);
        if (organization_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing organization scope"
            );
            error.send(sink);
            return;
        }

        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{merchant_id});
        ctx.claims.insert_or_assign("organization_id", std::string{organization_id});
        ctx.claims.insert_or_assign("resource_type", std::string{resource_type});
        m_runtime.generate(ctx).handle(sink);
    }

    void handle_upload_hierarchical_resource(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // web::Path<ResourceId> is extracted by the framework (routes:117); the
        // local extractor validates the same alphanumeric/length shape.
        const auto resource_id =
            m_input_support.resource_id_from_path(req, "apple_pay_certificate");
        if (!resource_id.has_value() || resource_id->empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing resource id in path"
            );
            error.send(sink);
            return;
        }

        // routes:48-71 — content-type aware body parse: JSON object or raw DER
        // bytes that are base64-encoded into the request's `certificate` field.
        auto parsed = core::behavior::Serializer<::app::dto::hierarchical_resources::ListHierarchicalResourcesRequest>::pass(req, sink);
        if (!parsed) return;
        const auto resource_type = parsed->get_resource_type().get_value();
        if (!m_input_support.valid_resource_type(resource_type)) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid resource type"
            );
            error.send(sink);
            return;
        }

        const auto profile_id = m_input_support.mandatory_profile_id_from_header(req);
        if (!profile_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing X-Profile-Id header"
            );
            error.send(sink);
            return;
        }
        const auto auth_error = m_support.authorize(req, "list_hierarchical_resources", {});
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = m_input_support.resolve_merchant_id(req, auth);
        if (!m_input_support.resolve_profile_from_header(*profile_id, merchant_id).has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "profile not found for merchant"
            );
            error.send(sink);
            return;
        }
        const auto organization_id = m_input_support.resolve_scope_id(req, auth, merchant_id);

        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{merchant_id});
        ctx.claims.insert_or_assign("organization_id", std::string{organization_id});
        m_runtime.list(*parsed, ctx).handle(sink);
    }

    void handle_link_hierarchical_resource(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        const auto resource_id = m_input_support.resource_id_from_path(req, "link");
        if (!resource_id.has_value() || resource_id->empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing resource id in path"
            );
            error.send(sink);
            return;
        }

        auto parsed = core::behavior::Serializer<::app::dto::hierarchical_resources::LinkHierarchicalResourceRequest>::pass(req, sink);
        if (!parsed) return;
        const auto requestor_type = parsed->get_requestor_type().get_value();
        const auto& requestor_id = parsed->get_requestor_id();
        if (!m_input_support.valid_requestor_type(requestor_type) || requestor_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "requestor_type and requestor_id are required"
            );
            error.send(sink);
            return;
        }

        const auto profile_id = m_input_support.mandatory_profile_id_from_header(req);
        if (!profile_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing X-Profile-Id header"
            );
            error.send(sink);
            return;
        }
        const auto auth_error = m_support.authorize(req, "link_hierarchical_resource", {});
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = m_input_support.resolve_merchant_id(req, auth);
        if (!m_input_support.resolve_profile_from_header(*profile_id, merchant_id).has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "profile not found for merchant"
            );
            error.send(sink);
            return;
        }
        const auto organization_id = m_input_support.resolve_scope_id(req, auth, merchant_id);

        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("resource_id", std::string{*resource_id});
        ctx.claims.insert_or_assign("requestor_type", std::string{requestor_type});
        ctx.claims.insert_or_assign("requestor_id", std::string{requestor_id});
        ctx.claims.insert_or_assign("merchant_id", std::string{merchant_id});
        ctx.claims.insert_or_assign("organization_id", std::string{organization_id});
        m_runtime.link(ctx).handle(sink);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Upstream scope registration uses web::resource("") for generate
        // (routes/app.rs:2282-2284); an empty segment is the exact match.
        Router{router, "hierarchical_resources"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_generate_hierarchical_resource(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch generate_hierarchical_resource")
            )
            .add_route(
                Route{"list"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_list_hierarchical_resources(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_hierarchical_resources")
            )
            .add_router(
                Router{router, "apple_pay_certificate"}.add_route(
                    Route{":resource_id"}
                        .put(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_upload_hierarchical_resource(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch upload_hierarchical_resource")
                )
            )
            .add_router(
                Router{router, ":resource_id"}.add_route(
                    Route{"link"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_link_hierarchical_resource(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch link_hierarchical_resource")
                )
            );
    }

private:
    HierarchicalResourcesRuntime m_runtime;
    HierarchicalResourcesInputSupport m_input_support;

    HierarchicalResourcesPolicy m_policy{m_input_support};

    HierarchicalResourcesRequestSupport m_support{m_policy};
};

} // namespace app::handlers::hierarchical_resources
