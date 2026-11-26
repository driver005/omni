export module app_user:user_role_runtime_get_authorization_info;

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
import app_events;
import app_keystore;

import :user_role_input;
import :user_role_result;
import :user_role_runtime_common;

export namespace app::handlers::user_role {

class GetAuthorizationInfoRuntime
{
public:
    explicit GetAuthorizationInfoRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetAuthorizationInfoResult, core::error::AppError>
    get_authorization_info(const GetAuthorizationInfoInput& input) const
    {
        (void)input;
        auto groups = m_support.group_authorization_info();
        m_support.track_flow("get_authorization_info", "user_role.get_authorization_info");
        return GetAuthorizationInfoResult{std::move(groups)};
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
