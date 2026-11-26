export module app_user:profiles_runtime;

import std;
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

    [[nodiscard]] std::expected<ProfileCreateResult, core::error::AppError>
    create(const ProfileCreateInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<ProfileRetrieveResult, core::error::AppError>
    retrieve(const ProfileRetrieveInput& input) const
    {
        return m_retrieve.retrieve(input);
    }

    [[nodiscard]] std::expected<ProfileUpdateResult, core::error::AppError>
    update(const ProfileUpdateInput& input) const
    {
        return m_update.update(input);
    }

    [[nodiscard]] std::expected<ProfileDeleteResult, core::error::AppError>
    remove(const ProfileDeleteInput& input) const
    {
        return m_delete.remove(input);
    }

    [[nodiscard]] std::expected<ProfilesListResult, core::error::AppError>
    list(const ProfilesListInput& input) const
    {
        return m_list.list(input);
    }

    [[nodiscard]] std::expected<ProfilesListAtProfileLevelResult, core::error::AppError>
    list_at_profile_level(const ProfilesListAtProfileLevelInput& input) const
    {
        return m_list_at_profile_level.list(input);
    }

    [[nodiscard]] std::expected<ToggleConnectorAgnosticMitResult, core::error::AppError>
    toggle_connector_agnostic_mit(const ToggleConnectorAgnosticMitInput& input) const
    {
        return m_toggle_connector_agnostic_mit.toggle(input);
    }

    [[nodiscard]] std::expected<ToggleExtendedCardInfoResult, core::error::AppError>
    toggle_extended_card_info(const ToggleExtendedCardInfoInput& input) const
    {
        return m_toggle_extended_card_info.toggle(input);
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
