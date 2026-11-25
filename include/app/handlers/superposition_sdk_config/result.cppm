export module app_handlers_superposition_sdk_config:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::superposition_sdk_config {

// Both routes encode the exact SuperPositionConfigResponse DTO the old core built and
// encoded, so field names, order and wrapper types stay byte-identical. The Serializable
// specialization reuses the DTO's FieldDesc tuple; those descriptors apply to the derived
// object just as they do to the base object.

class GetSdkConfigResult : public app::dto::superposition_sdk_config::SuperPositionConfigResponse
{
public:
    GetSdkConfigResult() = default;

    explicit GetSdkConfigResult(
        app::dto::superposition_sdk_config::SuperPositionConfigResponse value
    ) noexcept :
        SuperPositionConfigResponse{std::move(value)}
    {
    }
};

class GetProfileSdkConfigResult :
    public app::dto::superposition_sdk_config::SuperPositionConfigResponse
{
public:
    GetProfileSdkConfigResult() = default;

    explicit GetProfileSdkConfigResult(
        app::dto::superposition_sdk_config::SuperPositionConfigResponse value
    ) noexcept :
        SuperPositionConfigResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::superposition_sdk_config

template<>
struct serde::Serializable<app::handlers::superposition_sdk_config::GetSdkConfigResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            app::dto::superposition_sdk_config::SuperPositionConfigResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_sdk_config_result";
    }
};

template<>
struct serde::Serializable<app::handlers::superposition_sdk_config::GetProfileSdkConfigResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            app::dto::superposition_sdk_config::SuperPositionConfigResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_profile_sdk_config_result";
    }
};
