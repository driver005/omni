export module app_handlers_profiles:runtime_toggle_extended_card_info;

import std;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_metrics;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::profiles {

// core/admin.rs:4900-4941 — extended_card_info_toggle.
class ToggleExtendedCardInfoRuntime
{
public:
    explicit ToggleExtendedCardInfoRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ToggleExtendedCardInfoResult, core::error::AppError>
    toggle(const ToggleExtendedCardInfoInput& input) const
    {
        // core/admin.rs:4906-4912 — key store by merchant id.
        if (!m_support.get_merchant_key_store_by_merchant_id(
                input.merchant_id,
                m_support.master_key()
            )) {
            return std::unexpected(m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "merchant account not found"
            ));
        }
        // core/admin.rs:4914-4920 — find_business_profile_by_profile_id.
        const auto stored = m_support.find_business_profile_by_profile_id(input.profile_id);
        if (!stored) {
            return std::unexpected(m_support.profile_not_found(input.profile_id));
        }
        // core/admin.rs:4922-4938 — conditional ExtendedCardInfoUpdate. The
        // upstream domain flag has no ProfileResponse field on the wire either
        // (admin.rs:2838-3062), so the persisted scoped config row is its
        // in-process representation; the profile-owned wire type keeps only
        // the upstream-exposed `extended_card_info_config` field.
        const auto key = m_support.card_config_key(input.profile_id);
        const auto current = m_context.repository().get_config(key);
        if (!current || *current != (input.choice.get_enabled() ? "true" : "false")) {
            m_context.repository().set_config(key, input.choice.get_enabled() ? "true" : "false");
            m_support.emit_event("toggle_extended_card_info", serde::Ser::encode_json(*stored));
        }
        m_context.metrics().record_flow("profiles.toggle_extended_card_info");
        m_context.metrics().record_flow("Flow::ToggleExtendedCardInfo");
        m_context.metrics().counter("profiles_toggle_extended_card_info").increment();
        return ToggleExtendedCardInfoResult{input.choice};
    }

private:
    app::context::AppContext& m_context;
    ProfilesRuntimeSupport m_support;
};

} // namespace app::handlers::profiles
