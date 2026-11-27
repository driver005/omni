export module app_user:user_runtime_switch_org_for_user;

import std;
import core_behavior;
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
import app_locking;
import app_services;

import :user_input;
import :user_result;
import :user_runtime_common;

export namespace app::handlers::user {

class SwitchOrgForUserRuntime
{
public:
    explicit SwitchOrgForUserRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    switch_org_for_user(const ::app::model::user::SwitchOrganizationRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        std::string set_cookie{};


        const auto owner = ctx_token(ctx).value();
        const auto new_org = request.get_org_id().get_value();
        for (auto& role: m_context.repository().list_user_roles()) {
            if (role.get_user_id() == owner) {
                role.set_organization_id(new_org);
                m_context.repository().update_user_role(role);
            }
        }
        const auto switched_token = m_support.user_signer().sign(
            std::format(R"({{"sub":"{}","org_id":"{}"}})", owner, new_org)
        );
        set_cookie = "user_token=" + switched_token + "; HttpOnly; Path=/";
        m_support.publish_identity_event("user.org_switched", owner);

        m_context.metrics().record_flow("switch_org_for_user");
        m_context.metrics().counter("user.switch_org_for_user").increment();

        using Response = ::app::model::user::TokenResponse;
        Response response{};
        ::app::model::user::TokenPurposeValue purpose{};
        purpose.set_value("LOGIN");
        response.set_token(switched_token);
        response.set_token_type(purpose);
        return std::pair{ResponseDocument::from(response), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
