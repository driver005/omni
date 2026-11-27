export module app_user:user_role_runtime_create_role;

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

class CreateRoleRuntime
{
public:
    explicit CreateRoleRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<RoleInfoWithGroupsWire>
    create_role(const CreateRoleInputWire& request,
        const ::core::behavior::CallContext& ctx) const
    {
        const auto requestor = m_support.role_info_from_id(identity_from_context(ctx), identity_from_context(ctx).role_id);
        if (!requestor) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleId")
            };
        }
        if (const auto err =
                m_support.validate_create_role(request, identity_from_context(ctx), *requestor)) {
            return std::unexpected{*err};
        }
        const auto merchant =
            m_context.repository().find_merchant_account_by_string(identity_from_context(ctx).merchant_id);
        if (!merchant) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "MerchantIdNotFound")
            };
        }
        const auto product_type = m_support.merchant_product_type(*merchant);
        if (const auto err =
                role_tables::validate_role_groups(request.get_groups(), product_type)) {
            return std::unexpected{*err};
        }
        const auto role_entity = m_support.normalize_entity(request.get_entity_type()).empty()
                                     ? std::string{"merchant"}
                                     : m_support.normalize_entity(request.get_entity_type());
        if (const auto err = m_support.validate_role_name(
                role_tables::lowercased(request.get_role_name()),
                identity_from_context(ctx),
                role_entity
            )) {
            return std::unexpected{*err};
        }

        ::app::store::roles::RoleRecord record{};
        record.set_role_id(m_context.ids().payment_id("role_"));
        record.set_role_name(role_tables::lowercased(request.get_role_name()));
        record.set_entity_type(role_entity);
        record.set_merchant_id(
            role_entity == "merchant" || role_entity == "profile" ? identity_from_context(ctx).merchant_id
                                                                  : std::string{}
        );
        record.set_organization_id(identity_from_context(ctx).org_id);
        record.set_profile_id(role_entity == "profile" ? identity_from_context(ctx).profile_id : std::string{});
        record.set_is_internal(0);
        record.set_status("active");
        record.set_created_at(m_support.now_string());
        const auto saved = m_context.repository().create_role(std::move(record));
        if (!saved) {
            return std::unexpected{saved.error()};
        }
        m_support.store_role_metadata(
            saved->get_role_id(),
            request.get_groups(),
            request.get_role_scope(),
            product_type
        );

        RoleInfoWithGroupsWire response{};
        response.set_role_id(saved->get_role_id());
        response.set_groups(request.get_groups());
        response.set_role_name(saved->get_role_name());
        response.set_role_scope(request.get_role_scope());
        response.set_entity_type(saved->get_entity_type());
        m_support.track_flow("create_role", "user_role.create_role");
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
