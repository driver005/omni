export module app_user:profiles_runtime_update;

import std;
import core_behavior;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_metrics;

import :profiles_input;
import :profiles_result;
import :profiles_runtime_common;

export namespace app::handlers::profiles {

// core/admin.rs:4673-4744 — update_profile.
class ProfileUpdateRuntime
{
public:
    explicit ProfileUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<ProfileWireResponse>
    update(const ::app::dto::admin::ProfileUpdate& request,
        const ::core::behavior::CallContext& ctx) const
    {
        auto repository = m_context.repository();
        // core/admin.rs:4681-4685 — find_merchant_account_by_merchant_id.
        auto account = repository.find_merchant_account_by_string(ctx_string(ctx,"merchant_id"));
        if (!account) {
            return std::unexpected(m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "merchant account not found"
            ));
        }
        // core/admin.rs:4686-4690.
        if (const auto error = m_support.validate_external_vault_config_for_merchant_account_type(
                account->get_merchant_account_type(),
                request.get_is_external_vault_enabled(),
                request.get_external_vault_connector_details().has_value()
            )) {
            return std::unexpected(*error);
        }
        // core/admin.rs:4692-4698 — find_business_profile_by_merchant_id_profile_id.
        const auto stored = m_support.find_business_profile_by_merchant_id_profile_id(
            ctx_string(ctx,"merchant_id"),
            ctx_string(ctx,"profile_id")
        );
        if (!stored) {
            return std::unexpected(m_support.profile_not_found(ctx_string(ctx,"profile_id")));
        }
        // core/admin.rs:4700-4725 — DE-cut-over profiles reject direct routing writes.
        if ((request.get_routing_algorithm() ||
             request.get_payout_routing_algorithm()) &&
            m_support.decision_engine_routing_effective(ctx_string(ctx,"profile_id"))) {
            return std::unexpected(m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "routing_algorithm cannot be set directly for a profile routed by the Decision "
                "Engine; use the routing APIs"
            ));
        }
        // core/admin.rs:4727-4729 — ProfileUpdateBridge::get_update_profile_object.
        auto updated = m_support.get_update_profile_object(*stored, request);
        if (!updated) {
            return std::unexpected(updated.error());
        }
        // core/admin.rs:4731-4737 — update_profile_by_profile_id.
        if (!m_support
                 .update_profile_by_profile_id(ctx_string(ctx,"profile_id"), ctx_string(ctx,"merchant_id"), *updated)) {
            return std::unexpected(m_support.profile_not_found(ctx_string(ctx,"profile_id")));
        }
        // core/admin.rs:4739-4742 — ProfileResponse::foreign_try_from(updated row).
        const auto text = serde::Ser::encode_json(*updated);
        m_support.emit_event("update", text);
        m_context.metrics().record_flow("profiles.update");
        m_context.metrics().record_flow("Flow::ProfileUpdate");
        m_context.metrics().counter("profiles_update").increment();
        return std::move(*updated);
    }

private:
    ::app::context::AppContext& m_context;
    ProfilesRuntimeSupport m_support;
};

} // namespace app::handlers::profiles
