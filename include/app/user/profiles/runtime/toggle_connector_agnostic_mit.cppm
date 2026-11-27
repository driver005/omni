export module app_user:profiles_runtime_toggle_connector_agnostic_mit;

import std;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_metrics;

import :profiles_input;
import :profiles_result;
import :profiles_runtime_common;

export namespace app::handlers::profiles {

// core/admin.rs:4943-4993 — connector_agnostic_mit_toggle.
class ToggleConnectorAgnosticMitRuntime
{
public:
    explicit ToggleConnectorAgnosticMitRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::ConnectorAgnosticMitChoice>
        toggle(const ::app::dto::admin::ConnectorAgnosticMitChoice& request,
        const ::core::behavior::CallContext& ctx) const
    {
        // core/admin.rs:4950-4992 — processor merchant must equal the route
        // merchant; the header/route binding is enforced by
        // authorize_profile_operation and the ownership check below.
        auto repository = m_context.repository();
        auto account = repository.find_merchant_account_by_string(ctx_string(ctx,"merchant_id"));
        if (!account) {
            return std::unexpected(m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "merchant account not found"
            ));
        }
        const auto stored = m_support.find_business_profile_by_merchant_id_profile_id(
            ctx_string(ctx,"merchant_id"),
            ctx_string(ctx,"profile_id")
        );
        if (!stored) {
            return std::unexpected(m_support.profile_not_found(ctx_string(ctx,"profile_id")));
        }
        // core/admin.rs:4962-4966 — AccessForbidden on owner mismatch.
        if (stored->get_merchant_id() != ctx_string(ctx,"merchant_id") && !ctx_string(ctx,"merchant_id").empty()) {
            return std::unexpected(m_support.make_error(
                core::error::AppErrorKind::FORBIDDEN,
                std::format("Access forbidden for resource {}", ctx_string(ctx,"profile_id"))
            ));
        }
        // core/admin.rs:4968-4982 — conditional ConnectorAgnosticMitUpdate.
        if (stored->get_is_connector_agnostic_mit_enabled().value_or(false) !=
            request.get_enabled()) {
            auto merged = *stored;
            merged.set_is_connector_agnostic_mit_enabled(request.get_enabled());
            if (!m_support
                     .update_profile_by_profile_id(ctx_string(ctx,"profile_id"), ctx_string(ctx,"merchant_id"), merged)) {
                return std::unexpected(m_support.profile_not_found(ctx_string(ctx,"profile_id")));
            }
            m_support.emit_event("toggle_connector_agnostic_mit", serde::Ser::encode_json(merged));
        }
        m_context.metrics().record_flow("profiles.toggle_connector_agnostic_mit");
        m_context.metrics().record_flow("Flow::ToggleConnectorAgnosticMit");
        m_context.metrics().counter("profiles_toggle_connector_agnostic_mit").increment();
        return request;
    }

private:
    ::app::context::AppContext& m_context;
    ProfilesRuntimeSupport m_support;
};

} // namespace app::handlers::profiles
