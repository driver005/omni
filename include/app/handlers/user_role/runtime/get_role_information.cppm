export module app_handlers_user_role:runtime_get_role_information;

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

class GetRoleInformationRuntime
{
public:
    explicit GetRoleInformationRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetRoleInformationResult, core::error::AppError>
    get_role_information(const GetRoleInformationInput& input) const
    {
        (void)input;
        auto parents = m_support.parent_authorization_info();
        m_support.track_flow("get_role_information", "user_role.get_role_information");
        return GetRoleInformationResult{std::move(parents)};
    }

private:
    app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
