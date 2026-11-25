export module app_handlers_hierarchical_resources:handler;

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
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :input;
import :result;
import :runtime;

export namespace app::handlers::hierarchical_resources {

class HierarchicalResourcesRouteHandlers
{
public:
    explicit HierarchicalResourcesRouteHandlers(app::context::AppContext& context) noexcept :
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
        // Upstream: web::Json<GenerateHierarchicalResourceRequest> is decoded by the
        // framework before the handler (routes:77); a bad body is a 400.
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = app::dto::hierarchical_resources::GenerateHierarchicalResourceRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto resource_type = parsed->get_resource_type().get_value();
        if (!m_input_support.valid_resource_type(resource_type)) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid resource type"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // routes:81-84 — mandatory_profile_id_from_header runs before server_wrap.
        const auto profile_id = m_input_support.mandatory_profile_id_from_header(req);
        if (!profile_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing X-Profile-Id header"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto auth_error =
            m_input_support.authorize_request(req, "generate_hierarchical_resource");
        if (auth_error) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = m_input_support.resolve_merchant_id(req, auth);
        // routes:93 — resolve_profile_from_header: scoped fetch is the auth check.
        if (!m_input_support.resolve_profile_from_header(*profile_id, merchant_id).has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "profile not found for merchant"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto organization_id = m_input_support.resolve_scope_id(req, auth, merchant_id);
        if (organization_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing organization scope"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        GenerateHierarchicalResourceInput input{merchant_id, organization_id, resource_type};
        auto result = m_runtime.generate(input);
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

    void handle_upload_hierarchical_resource(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // web::Path<ResourceId> is extracted by the framework (routes:117); the
        // local extractor validates the same alphanumeric/length shape.
        const auto resource_id =
            m_input_support.resource_id_from_path(req, "apple_pay_certificate");
        if (!resource_id.has_value() || resource_id->empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing resource id in path"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // routes:48-71 — content-type aware body parse: JSON object or raw DER
        // bytes that are base64-encoded into the request's `certificate` field.
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        auto parsed_body = m_input_support.parse_upload_certificate_body(req, body);
        if (!parsed_body) {
            const auto& error = parsed_body.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto profile_id = m_input_support.mandatory_profile_id_from_header(req);
        if (!profile_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing X-Profile-Id header"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto auth_error =
            m_input_support.authorize_request(req, "upload_hierarchical_resource");
        if (auth_error) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = m_input_support.resolve_merchant_id(req, auth);
        if (!m_input_support.resolve_profile_from_header(*profile_id, merchant_id).has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "profile not found for merchant"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto organization_id = m_input_support.resolve_scope_id(req, auth, merchant_id);

        UploadHierarchicalResourceInput
            input{*resource_id, parsed_body->get_certificate(), merchant_id, organization_id};
        auto result = m_runtime.upload(input);
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

    void handle_list_hierarchical_resources(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = app::dto::hierarchical_resources::ListHierarchicalResourcesRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto resource_type = parsed->get_resource_type().get_value();
        if (!m_input_support.valid_resource_type(resource_type)) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid resource type"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto profile_id = m_input_support.mandatory_profile_id_from_header(req);
        if (!profile_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing X-Profile-Id header"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto auth_error =
            m_input_support.authorize_request(req, "list_hierarchical_resources");
        if (auth_error) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = m_input_support.resolve_merchant_id(req, auth);
        if (!m_input_support.resolve_profile_from_header(*profile_id, merchant_id).has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "profile not found for merchant"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto organization_id = m_input_support.resolve_scope_id(req, auth, merchant_id);

        ListHierarchicalResourcesInput input{*parsed, merchant_id, organization_id};
        auto result = m_runtime.list(input);
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

    void handle_link_hierarchical_resource(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto resource_id = m_input_support.resource_id_from_path(req, "link");
        if (!resource_id.has_value() || resource_id->empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing resource id in path"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = app::dto::hierarchical_resources::LinkHierarchicalResourceRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto requestor_type = parsed->get_requestor_type().get_value();
        const auto& requestor_id = parsed->get_requestor_id();
        if (!m_input_support.valid_requestor_type(requestor_type) || requestor_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "requestor_type and requestor_id are required"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto profile_id = m_input_support.mandatory_profile_id_from_header(req);
        if (!profile_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing X-Profile-Id header"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto auth_error =
            m_input_support.authorize_request(req, "link_hierarchical_resource");
        if (auth_error) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = m_input_support.resolve_merchant_id(req, auth);
        if (!m_input_support.resolve_profile_from_header(*profile_id, merchant_id).has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "profile not found for merchant"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto organization_id = m_input_support.resolve_scope_id(req, auth, merchant_id);

        LinkHierarchicalResourceInput
            input{*resource_id, requestor_type, requestor_id, merchant_id, organization_id};
        auto result = m_runtime.link(input);
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

    void install(core::router::RouterContext<>& router)
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
};

} // namespace app::handlers::hierarchical_resources
