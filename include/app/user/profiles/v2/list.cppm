export module app_user:profiles_handlers_v2_list;

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

import :profiles_input;
import :profiles_result;
import :profiles_runtime;

export namespace app::handlers::profiles {

/**
 * v2 list adapter for `v2/merchant-accounts/:id/profiles` (upstream profiles.rs:438).
 */
class ProfilesHandlersV2List
{
public:
    ProfilesHandlersV2List(
        ::app::context::AppContext& context,
        ProfilesRuntime& runtime,
        ProfilesInputSupport& input_support,
        ProfilesRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support}
    {
    }

    void handle_profiles_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support.authorize(req, "profiles_list", {})) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        // profiles.rs:402-406 (v1) / :438 (v2) — list_profile(merchant, None).
        ProfilesListInput input{m_input_support.merchant_from_request(req), std::nullopt};
        auto result = m_runtime.list(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        auto encoded = serde::Serializer<std::vector<ProfileWireResponse>>::serialize_values(
            *result,
            kProfileWireFields
        );
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_body(serde::Ser::serialize_raw("application/json", *encoded));
        send();
    }

private:
    ::app::context::AppContext& m_context;
    ProfilesRuntime& m_runtime;
    ProfilesInputSupport& m_input_support;
    ProfilesRequestSupport& m_request_support;
};

} // namespace app::handlers::profiles
