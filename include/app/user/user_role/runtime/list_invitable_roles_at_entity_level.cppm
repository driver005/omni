export module app_user:user_role_runtime_list_invitable_roles_at_entity_level;

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

class ListInvitableRolesAtEntityLevelRuntime
{
public:
    explicit ListInvitableRolesAtEntityLevelRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_invitable_roles_at_entity_level(const ::core::behavior::CallContext& ctx) const
    {
        const auto requestor = m_support.role_info_from_id(
            identity_from_context(ctx),
            identity_from_context(ctx).role_id
        );
        if (!requestor) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INTERNAL, "InvalidRoleId")
            };
        }
        const auto entity_type = m_support.normalize_entity(ctx_string(ctx, "entity_type"));
        if (entity_type.empty()) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "entity_type required"
            )};
        }
        if (role_tables::entity_rank(entity_type) >
            role_tables::entity_rank(requestor->entity_type)) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::FORBIDDEN,
                "User is attempting to request list roles above the current entity level"
            )};
        }
        std::string product_type{};
        if (entity_type == "merchant" || entity_type == "profile") {
            const auto merchant = m_context.repository().find_merchant_account_by_string(
                identity_from_context(ctx).merchant_id
            );
            if (merchant) {
                product_type = m_support.merchant_product_type(*merchant);
            }
        }
        auto roles = m_support.roles_for_entity(
            identity_from_context(ctx),
            requestor->entity_type,
            entity_type,
            true
        );
        std::vector<MinimalRoleInfoWire> response{};
        for (const auto& role: roles) {
            if (role.entity_type != entity_type || !role.is_invitable) {
                continue;
            }
            if (!product_type.empty() && !role.effective_product_type().empty() &&
                role.effective_product_type() != product_type) {
                continue;
            }
            MinimalRoleInfoWire item{};
            item.set_role_id(role.role_id);
            item.set_role_name(role.role_name);
            response.push_back(std::move(item));
        }
        m_support.track_flow(
            "list_invitable_roles_at_entity_level",
            "user_role.list_invitable_roles_at_entity_level"
        );
        return to_document_array(response, kMinimalRoleInfoWireFields);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
