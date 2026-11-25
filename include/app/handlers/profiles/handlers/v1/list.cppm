export module app_handlers_profiles:handlers_v1_list;

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
 * v1 list adapters: merchant-level list and profile-level list (upstream profiles.rs:402-406,
 * :471-476).
 */
class ProfilesHandlersV1List
{
public:
    ProfilesHandlersV1List(
        app::context::AppContext& context,
        ProfilesRuntime& runtime,
        ProfilesInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_profiles_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize_profile_operation(req, "profiles_list")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        // profiles.rs:402-406 (v1) / :438 (v2) — list_profile(merchant, None).
        ProfilesListInput input{m_input_support.merchant_from_request(req), std::nullopt};
        auto result = m_runtime.list(input);
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

    void handle_profiles_list_at_profile_level(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize_profile_operation(
                req,
                "profiles_list_at_profile_level"
            )) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        // profiles.rs:471-476 — list_profile(merchant,
        // auth.profile.map(|p| vec![p.get_id().clone()])).
        ProfilesListAtProfileLevelInput input{
            m_input_support.merchant_from_request(req),
            m_input_support.profile_filter_from_auth(req)
        };
        auto result = m_runtime.list_at_profile_level(input);
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

private:
    app::context::AppContext& m_context;
    ProfilesRuntime& m_runtime;
    ProfilesInputSupport& m_input_support;
};

} // namespace app::handlers::profiles
