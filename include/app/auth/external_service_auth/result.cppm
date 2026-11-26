export module app_auth:external_service_auth_result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::external_service_auth {

// The wire fields inherit from the exact typed DTO the old core encoded, so
// field names, order and wrapper types stay byte-identical. The Serializable
// specialization reuses the DTO's FieldDesc tuple; those descriptors apply to
// the derived object just as they do to the base object.
class ValidateTokenResult : public ::app::dto::external_service_auth::ExternalVerifyTokenResponse
{
public:
    ValidateTokenResult() = default;

    explicit ValidateTokenResult(
        ::app::dto::external_service_auth::ExternalVerifyTokenResponse value
    ) noexcept :
        ExternalVerifyTokenResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::external_service_auth

template<>
struct serde::Serializable<::app::handlers::external_service_auth::ValidateTokenResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            ::app::dto::external_service_auth::ExternalVerifyTokenResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "validate_token_result";
    }
};
