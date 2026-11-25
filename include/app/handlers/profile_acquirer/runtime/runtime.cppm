export module app_handlers_profile_acquirer:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_create;
import :runtime_update;

export namespace app::handlers::profile_acquirer {

class ProfileAcquirerRuntime
{
public:
    explicit ProfileAcquirerRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_update{context}
    {
    }

    [[nodiscard]] std::expected<CreateProfileAcquirerResult, core::error::AppError>
    create(const CreateProfileAcquirerInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<ProfileAcquirerUpdateResult, core::error::AppError>
    update(const ProfileAcquirerUpdateInput& input) const
    {
        return m_update.update(input);
    }

private:
    app::context::AppContext& m_context;
    CreateProfileAcquirerRuntime m_create;
    ProfileAcquirerUpdateRuntime m_update;
};

} // namespace app::handlers::profile_acquirer
