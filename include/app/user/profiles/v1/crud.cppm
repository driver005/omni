export module app_user:profiles_handlers_v1_crud;

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

import :profiles_input;
import :profiles_result;
import :profiles_runtime;

export namespace app::handlers::profiles {

/**
 * v1 CRUD adapters for `account/:account_id/business_profile`. One thin adapter per operation;
 * runtime and input helper are owned once by the facade and shared by reference.
 */
class ProfilesHandlersV1Crud
{
public:
    ProfilesHandlersV1Crud(
        ::app::context::AppContext& context,
        ProfilesRuntime& runtime,
        ProfilesInputSupport& input_support,
        ProfilesRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_support{request_support}
    {
    }

    void handle_profile_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "profile_create", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<::app::dto::admin::ProfileCreate>::pass(req, sink);
        if (!parsed) return;
        // profiles.rs:22-45 (v1) / :79-90 (v2) — route-layer request validation.
        if (const auto error = m_input_support.validate_profile_create_request(*parsed)) {
            const auto& e = *error;
            e.send(sink);
            return;
        }
        // profiles.rs:21 / :76 — merchant id from route (v1) or header (v2).
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{m_input_support.merchant_from_request(req)});
        ctx.claims.insert_or_assign("v2_route", m_input_support.is_v2(req) ? "true" : "false");
        m_runtime.create(*parsed, ctx).handle(sink);
    }

    void handle_profile_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "profile_retrieve", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const std::string profile_id = m_input_support.profile_id_from_request(req);
        if (profile_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing profile id"
            );
            error.send(sink);
            return;
        }
        // profiles.rs:149-161 (v1) / :203-205 (v2) — retrieve with the
        // authenticated processor merchant (route for v1, header for v2).
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("profile_id", std::string{profile_id});
        ctx.claims.insert_or_assign("merchant_id", std::string{m_input_support.merchant_from_request(req)});
        m_runtime.retrieve(ctx).handle(sink);
    }

    void handle_profile_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "profile_update", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const std::string profile_id = m_input_support.profile_id_from_request(req);
        if (profile_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing profile id"
            );
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<::app::dto::admin::ProfileUpdate>::pass(req, sink);
        if (!parsed) return;
        // profiles.rs:235-259 (v1) / :314-325 (v2) — route-layer validation.
        if (const auto error = m_input_support.validate_profile_update_request(*parsed)) {
            const auto& e = *error;
            e.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("profile_id", std::string{profile_id});
        ctx.claims.insert_or_assign("merchant_id", std::string{m_input_support.merchant_from_request(req)});
        m_runtime.update(*parsed, ctx).handle(sink);
    }

    void handle_profile_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "profile_delete", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const std::string profile_id = m_input_support.profile_id_from_request(req);
        if (profile_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing profile id"
            );
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("profile_id", std::string{profile_id});
        ctx.claims.insert_or_assign("merchant_id", std::string{m_input_support.merchant_from_request(req)});
        m_runtime.remove(ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    ProfilesRuntime& m_runtime;
    ProfilesInputSupport& m_input_support;
    ProfilesRequestSupport& m_support;
};

} // namespace app::handlers::profiles
