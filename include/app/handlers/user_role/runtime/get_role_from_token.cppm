export module app_handlers_user_role:runtime_get_role_from_token;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::user_role {

class GetRoleFromTokenRuntime
{
public:
    explicit GetRoleFromTokenRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetRoleFromTokenResult, core::error::AppError>
    get_role_from_token(const GetRoleFromTokenInput& input) const
    {
        const auto role = m_support.role_info_from_id(input.identity, input.identity.role_id);
        if (!role) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleId")
            };
        }
        auto groups = role->permission_groups();
        m_support.track_flow("get_role_from_token", "user_role.get_role_from_token");
        return GetRoleFromTokenResult{std::move(groups)};
    }

private:
    app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
