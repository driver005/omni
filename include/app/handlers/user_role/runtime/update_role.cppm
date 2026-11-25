export module app_handlers_user_role:runtime_update_role;

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

class UpdateRoleRuntime
{
public:
    explicit UpdateRoleRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<UpdateRoleResult, core::error::AppError>
    update_role(const UpdateRoleInput& input) const
    {
        if (input.role_id.empty()) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleOperation")
            };
        }
        const auto role = m_support.role_info_from_id(input.identity, input.role_id);
        const auto requestor = m_support.role_info_from_id(input.identity, input.identity.role_id);
        if (!role || !requestor) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleOperation")
            };
        }
        const auto max_entity = std::max(
            role_tables::entity_rank(m_support.entity_from_scope(role->scope)),
            role_tables::entity_rank(role->entity_type)
        );
        if (role_tables::entity_rank(requestor->entity_type) < max_entity) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "InvalidRoleOperation"
            )};
        }
        auto updated_name = role->role_name;
        if (input.request.get_role_name().has_value()) {
            if (!role_tables::valid_role_name(*input.request.get_role_name())) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::VALIDATION_FAILED,
                    "RoleNameParsingError"
                )};
            }
            updated_name = role_tables::lowercased(*input.request.get_role_name());
            if (const auto err = m_support.validate_role_name(
                    updated_name,
                    input.identity,
                    role->entity_type,
                    role->role_id
                )) {
                return std::unexpected{*err};
            }
        }
        auto updated_groups = role->groups;
        if (input.request.get_groups().has_value()) {
            std::string product_type{};
            if (role->entity_type == "merchant" || role->entity_type == "profile") {
                const auto merchant = m_context.repository().find_merchant_account_by_string(
                    input.identity.merchant_id
                );
                if (!merchant) {
                    return std::unexpected{
                        m_support.error(core::error::AppErrorKind::NOT_FOUND, "MerchantIdNotFound")
                    };
                }
                product_type = m_support.merchant_product_type(*merchant);
            }
            if (const auto err =
                    role_tables::validate_role_groups(*input.request.get_groups(), product_type)) {
                return std::unexpected{*err};
            }
            updated_groups = *input.request.get_groups();
        }
        auto record = m_context.repository().find_role_by_string(input.role_id);
        if (!record) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleOperation")
            };
        }
        record->set_role_name(updated_name);
        m_context.repository().update_role(*record);
        m_support.store_role_metadata(
            role->role_id,
            updated_groups,
            role->scope,
            role->product_type_filter
        );

        RoleInfoWithGroupsWire response{};
        response.set_role_id(role->role_id);
        response.set_groups(role->permission_groups_from(updated_groups));
        response.set_role_name(updated_name);
        response.set_role_scope(role->scope);
        response.set_entity_type(role->entity_type);
        m_support.blacklist("role", role->role_id);
        m_support.track_flow("update_role", "user_role.update_role");
        return UpdateRoleResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
