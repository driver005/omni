export module app_handlers_user_role:runtime_list_invitable_roles_at_entity_level;

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

class ListInvitableRolesAtEntityLevelRuntime
{
public:
    explicit ListInvitableRolesAtEntityLevelRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ListInvitableRolesAtEntityLevelResult, core::error::AppError>
    list_invitable_roles_at_entity_level(const ListInvitableRolesAtEntityLevelInput& input) const
    {
        const auto requestor = m_support.role_info_from_id(input.identity, input.identity.role_id);
        if (!requestor) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INTERNAL, "InvalidRoleId")
            };
        }
        const auto entity_type = m_support.normalize_entity(input.entity_type);
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
            const auto merchant =
                m_context.repository().find_merchant_account_by_string(input.identity.merchant_id);
            if (merchant) {
                product_type = m_support.merchant_product_type(*merchant);
            }
        }
        auto roles =
            m_support.roles_for_entity(input.identity, requestor->entity_type, entity_type, true);
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
        return ListInvitableRolesAtEntityLevelResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
