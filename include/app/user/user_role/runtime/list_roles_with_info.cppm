export module app_user:user_role_runtime_list_roles_with_info;

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
import app_events;
import app_keystore;

import :user_role_input;
import :user_role_result;
import :user_role_runtime_common;

export namespace app::handlers::user_role {

class ListRolesWithInfoRuntime
{
public:
    explicit ListRolesWithInfoRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_roles_with_info(const ::core::behavior::CallContext& ctx) const
    {
        const auto requestor = m_support.role_info_from_id(identity_from_context(ctx), identity_from_context(ctx).role_id);
        if (!requestor || requestor->is_internal) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::FORBIDDEN, "InvalidRoleOperation")
            };
        }
        const auto entity_type = m_support.normalize_entity(ctx_string(ctx,"entity_type"));
        if (const auto err = m_support.get_min_entity(requestor->entity_type, entity_type)) {
            return std::unexpected{*err};
        }
        auto roles =
            m_support.roles_for_entity(identity_from_context(ctx), requestor->entity_type, entity_type, false);
        std::string product_type{};
        if (!entity_type.empty() && (entity_type == "merchant" || entity_type == "profile")) {
            const auto merchant =
                m_context.repository().find_merchant_account_by_string(identity_from_context(ctx).merchant_id);
            if (!merchant) {
                return std::unexpected{
                    m_support.error(core::error::AppErrorKind::NOT_FOUND, "MerchantIdNotFound")
                };
            }
            product_type = m_support.merchant_product_type(*merchant);
        }
        std::erase_if(
            roles,
            [&](const role_tables::RoleInfo& role)
            {
                if (!product_type.empty() && !role.effective_product_type().empty() &&
                    role.effective_product_type() != product_type) {
                    return true;
                }
                if (role_tables::entity_rank(requestor->entity_type) <
                    role_tables::entity_rank(role.entity_type)) {
                    return true;
                }
                return !entity_type.empty() && role.entity_type != entity_type;
            }
        );
        if (ctx_bool(ctx,"groups")) {
            std::vector<RoleInfoResponseWithParentsGroupWire> with_parents{};
            for (const auto& role: roles) {
                RoleInfoResponseWithParentsGroupWire item{};
                item.set_role_id(role.role_id);
                item.set_role_name(role.role_name);
                item.set_entity_type(role.entity_type);
                item.set_parent_groups(
                    role_tables::descriptions_for_groups(role.entity_type, role.permission_groups())
                );
                item.set_role_scope(role.scope);
                with_parents.push_back(std::move(item));
            }
            m_support.track_flow("list_roles_with_info", "user_role.list_roles_with_info");
            return ResponseDocument::from(with_parents);
        }
        std::vector<RoleInfoResponseNewWire> response{};
        for (const auto& role: roles) {
            RoleInfoResponseNewWire item{};
            item.set_role_id(role.role_id);
            item.set_role_name(role.role_name);
            item.set_entity_type(role.entity_type);
            item.set_groups(role.permission_groups());
            item.set_scope(role.scope);
            response.push_back(std::move(item));
        }
        m_support.track_flow("list_roles_with_info", "user_role.list_roles_with_info");
        return ResponseDocument::from(response);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
