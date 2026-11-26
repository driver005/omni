export module app_user:user_runtime_list_user_roles_details;

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
import app_locking;
import app_services;

import :user_input;
import :user_result;
import :user_runtime_common;

export namespace app::handlers::user {

class ListUserRolesDetailsRuntime
{
public:
    explicit ListUserRolesDetailsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ListUserRolesDetailsResult, core::error::AppError>
    list_user_roles_details(const ListUserRolesDetailsInput& input) const
    {
        std::string set_cookie{};


        // Port of core/user.rs:1901-2115: resolve the requested user by email and
        // emit a Vec<GetUserRoleDetailsResponseV2> built from persisted user roles.
        using Response = std::vector<::app::model::user::GetUserRoleDetailsResponseV2>;
        Response response{};
        const auto email = input.request.get_email().get_value();
        std::string target_user{};
        const auto index = m_context.cache().get("user.email_index:" + email);
        if (index.hit) {
            target_user = core::cache::value_as<std::string>(index.value, std::string{});
        }
        if (!email.empty() && target_user.empty()) {
            return std::unexpected{m_support.wire_error(
                "UR_23",
                "User Role Operation Not Supported",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }
        const auto org_id = input.request_context.org_id;
        const auto merchant_id = input.request_context.merchant_id;
        const auto profile_id = input.request_context.auth.profile_id();
        for (const auto& role: m_context.repository().list_user_roles()) {
            if (!target_user.empty() && role.get_user_id() != target_user) {
                continue;
            }
            if (!org_id.empty() && role.get_organization_id() != org_id) {
                continue;
            }
            if (!merchant_id.empty() && role.get_merchant_id() != merchant_id) {
                continue;
            }
            ::app::model::user::GetUserRoleDetailsResponseV2 item{};
            item.set_role_id(role.get_role_id());
            ::app::model::user::NameIdUnit org{};
            org.set_name(role.get_organization_id());
            org.set_id(role.get_organization_id());
            item.set_org(org);
            if (!role.get_merchant_id().empty()) {
                ::app::model::user::NameIdUnit merchant{};
                merchant.set_name(role.get_merchant_id());
                merchant.set_id(role.get_merchant_id());
                item.set_merchant(merchant);
            }
            if (!role.get_profile_id().empty()) {
                ::app::model::user::NameIdUnit profile{};
                profile.set_name(role.get_profile_id());
                profile.set_id(role.get_profile_id());
                item.set_profile(profile);
            }
            ::app::model::user::UserStatusValue status{};
            status.set_value(role.get_status());
            item.set_status(status);
            ::app::model::user::EntityTypeValue entity{};
            entity.set_value(
                role.get_profile_id().empty()
                    ? (role.get_merchant_id().empty() ? "organization" : "merchant")
                    : "profile"
            );
            item.set_entity_type(entity);
            item.set_role_name(role.get_role_id());
            response.push_back(std::move(item));
        }
        (void)profile_id;

        m_context.metrics().record_flow("list_user_roles_details");
        m_context.metrics().counter("user.list_user_roles_details").increment();

        auto document = ResponseDocument::from(response);
        return ListUserRolesDetailsResult{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
