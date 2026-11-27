export module app_user:profiles_runtime;

import std;
import core_behavior;
import core_error;
import app_context;

import :profiles_input;
import :profiles_result;
import :profiles_runtime_common;
import :profiles_runtime_create;
import :profiles_runtime_retrieve;
import :profiles_runtime_update;
import :profiles_runtime_delete;
import :profiles_runtime_list;
import :profiles_runtime_list_at_profile_level;
import :profiles_runtime_toggle_connector_agnostic_mit;
import :profiles_runtime_toggle_extended_card_info;

export namespace app::handlers::profiles {

class ProfilesRuntime
{
public:
    explicit ProfilesRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_retrieve{context},
        m_update{context},
        m_delete{context},
        m_list{context},
        m_list_at_profile_level{context},
        m_toggle_connector_agnostic_mit{context},
        m_toggle_extended_card_info{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<ProfileWireResponse> create(
        const ::app::dto::admin::ProfileCreate& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_create.create(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<ProfileWireResponse>
    retrieve(const ::core::behavior::CallContext& ctx) const
    {
        return m_retrieve.retrieve(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<ProfileWireResponse> update(
        const ::app::dto::admin::ProfileUpdate& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_update.update(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    remove(const ::core::behavior::CallContext& ctx) const
    {
        return m_delete.remove(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list(const ::core::behavior::CallContext& ctx) const
    {
        return m_list.list(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_at_profile_level(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_at_profile_level.list(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::ConnectorAgnosticMitChoice>
    toggle_connector_agnostic_mit(
        const ::app::dto::admin::ConnectorAgnosticMitChoice& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_toggle_connector_agnostic_mit.toggle(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::ExtendedCardInfoChoice>
    toggle_extended_card_info(
        const ::app::dto::admin::ExtendedCardInfoChoice& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_toggle_extended_card_info.toggle(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    ProfileCreateRuntime m_create;
    ProfileRetrieveRuntime m_retrieve;
    ProfileUpdateRuntime m_update;
    ProfileDeleteRuntime m_delete;
    ProfilesListRuntime m_list;
    ProfilesListAtProfileLevelRuntime m_list_at_profile_level;
    ToggleConnectorAgnosticMitRuntime m_toggle_connector_agnostic_mit;
    ToggleExtendedCardInfoRuntime m_toggle_extended_card_info;
};

} // namespace app::handlers::profiles
