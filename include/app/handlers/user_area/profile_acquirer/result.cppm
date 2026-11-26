export module app_user:profile_acquirer_result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::profile_acquirer {

// Both routes return ProfileAcquirerResponse from the old core. The result classes
// inherit the DTO's wire fields and their Serializable specialization reuses the
// DTO's FieldDesc tuple, so field names, order and wrapper types stay byte-identical
// to the old serde::Ser::encode_json(response) body.

class CreateProfileAcquirerResult : public ::app::dto::profile_acquirer::ProfileAcquirerResponse
{
public:
    CreateProfileAcquirerResult() = default;

    explicit CreateProfileAcquirerResult(
        ::app::dto::profile_acquirer::ProfileAcquirerResponse value
    ) noexcept :
        ProfileAcquirerResponse{std::move(value)}
    {
    }
};

class ProfileAcquirerUpdateResult : public ::app::dto::profile_acquirer::ProfileAcquirerResponse
{
public:
    ProfileAcquirerUpdateResult() = default;

    explicit ProfileAcquirerUpdateResult(
        ::app::dto::profile_acquirer::ProfileAcquirerResponse value
    ) noexcept :
        ProfileAcquirerResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::profile_acquirer

template<>
struct serde::Serializable<::app::handlers::profile_acquirer::CreateProfileAcquirerResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::profile_acquirer::ProfileAcquirerResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "create_profile_acquirer_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::profile_acquirer::ProfileAcquirerUpdateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::profile_acquirer::ProfileAcquirerResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "profile_acquirer_update_result";
    }
};
