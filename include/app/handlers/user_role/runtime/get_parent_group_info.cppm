export module app_handlers_user_role:runtime_get_parent_group_info;

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

class GetParentGroupInfoRuntime
{
public:
    explicit GetParentGroupInfoRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetParentGroupInfoResult, core::error::AppError>
    get_parent_group_info(const GetParentGroupInfoInput& input) const
    {
        const auto requestor = m_support.role_info_from_id(input.identity, input.identity.role_id);
        if (!requestor) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleId")
            };
        }
        const auto entity_type = m_support.normalize_entity(input.entity_type);
        const auto effective_entity = entity_type.empty() ? requestor->entity_type : entity_type;
        if (role_tables::entity_rank(requestor->entity_type) <
            role_tables::entity_rank(effective_entity)) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "InvalidRoleOperation"
            )};
        }
        std::vector<std::string> groups{};
        if (!input.product_type.empty()) {
            const auto categories = role_tables::accessible_product_categories(input.product_type);
            for (const auto& def: role_tables::perm_groups()) {
                if (std::find(categories.begin(), categories.end(), std::string{def.category}) !=
                    categories.end()) {
                    groups.emplace_back(def.name);
                }
            }
        } else {
            for (const auto& def: role_tables::perm_groups()) {
                groups.emplace_back(def.name);
            }
        }
        auto descriptions = role_tables::descriptions_for_groups(effective_entity, groups);
        m_support.track_flow("get_parent_group_info", "user_role.get_parent_group_info");
        return GetParentGroupInfoResult{std::move(descriptions)};
    }

private:
    app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
