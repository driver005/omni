export module app_handlers_profiles:handlers_v1_crud;

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

import :input;
import :result;
import :runtime;

export namespace app::handlers::profiles {

/**
 * v1 CRUD adapters for `account/:account_id/business_profile`. One thin adapter per operation;
 * runtime and input helper are owned once by the facade and shared by reference.
 */
class ProfilesHandlersV1Crud
{
public:
    ProfilesHandlersV1Crud(
        app::context::AppContext& context,
        ProfilesRuntime& runtime,
        ProfilesInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_profile_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize_profile_operation(req, "profile_create")) {
            const auto& error = *auth_error;
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
        using Input = app::dto::admin::ProfileCreate;
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
        // profiles.rs:22-45 (v1) / :79-90 (v2) — route-layer request validation.
        if (const auto error = m_input_support.validate_profile_create_request(*parsed)) {
            const auto& e = *error;
            res.set_status(e.get_status());
            res.set_body(serde::Ser::serialize("application/json", e));
            send();
            return;
        }
        // profiles.rs:21 / :76 — merchant id from route (v1) or header (v2).
        ProfileCreateInput input{
            *parsed,
            m_input_support.merchant_from_request(req),
            m_input_support.is_v2(req)
        };
        auto result = m_runtime.create(input);
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

    void handle_profile_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize_profile_operation(req, "profile_retrieve")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string profile_id = m_input_support.profile_id_from_request(req);
        if (profile_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing profile id"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        // profiles.rs:149-161 (v1) / :203-205 (v2) — retrieve with the
        // authenticated processor merchant (route for v1, header for v2).
        ProfileRetrieveInput input{profile_id, m_input_support.merchant_from_request(req)};
        auto result = m_runtime.retrieve(input);
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

    void handle_profile_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize_profile_operation(req, "profile_update")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string profile_id = m_input_support.profile_id_from_request(req);
        if (profile_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing profile id"
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
        using Input = app::dto::admin::ProfileUpdate;
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
        // profiles.rs:235-259 (v1) / :314-325 (v2) — route-layer validation.
        if (const auto error = m_input_support.validate_profile_update_request(*parsed)) {
            const auto& e = *error;
            res.set_status(e.get_status());
            res.set_body(serde::Ser::serialize("application/json", e));
            send();
            return;
        }
        ProfileUpdateInput input{profile_id, m_input_support.merchant_from_request(req), *parsed};
        auto result = m_runtime.update(input);
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

    void handle_profile_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize_profile_operation(req, "profile_delete")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string profile_id = m_input_support.profile_id_from_request(req);
        if (profile_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing profile id"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        ProfileDeleteInput input{profile_id, m_input_support.merchant_from_request(req)};
        auto result = m_runtime.remove(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        // core/admin.rs:4195 — Json(bool): literal true from the store delete result.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_value()));
        send();
    }

private:
    app::context::AppContext& m_context;
    ProfilesRuntime& m_runtime;
    ProfilesInputSupport& m_input_support;
};

} // namespace app::handlers::profiles
