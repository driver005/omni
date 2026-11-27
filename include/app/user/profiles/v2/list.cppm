export module app_user:profiles_handlers_v2_list;

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
        m_support{request_support}
    {
    }

    void handle_profiles_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "profiles_list", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        // profiles.rs:402-406 (v1) / :438 (v2) — list_profile(merchant, None).
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("merchant_id", std::string{m_input_support.merchant_from_request(req)});
        ctx.claims.insert_or_assign("profile_id_list", encode_ids(std::nullopt));
        m_runtime.list(ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    ProfilesRuntime& m_runtime;
    ProfilesInputSupport& m_input_support;
    ProfilesRequestSupport& m_support;
};

} // namespace app::handlers::profiles
