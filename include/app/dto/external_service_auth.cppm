module;
export module app_dto:external_service_auth;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::external_service_auth {

class ExternalSignoutTokenRequest
{
public:
    void set_token(std::string value) { token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    std::string token{};
};

class ExternalTokenResponse
{
public:
    void set_token(std::string value) { token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    std::string token{};
};

class ExternalVerifyTokenRequest
{
public:
    void set_token(std::string value) { token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    std::string token{};
};

enum class ExternalVerifyTokenResponse
{
    Hypersense,
    OfferEngine,
};

enum class ValidatingService
{
    OfferEngine,
};

class ValidateTokenRequest
{
public:
    void set_token(std::string value) { token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    void set_service(ValidatingService value) { service = std::move(value); }
    [[nodiscard]] const ValidatingService& get_service() const noexcept { return service; }
    std::string token{};
    ValidatingService service{};
};
} // namespace app::dto::external_service_auth

template<>
struct serde::Serializable<app::dto::external_service_auth::ExternalTokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token", &app::dto::external_service_auth::ExternalTokenResponse::get_token, &app::dto::external_service_auth::ExternalTokenResponse::set_token, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "external_token_response"; }
};

template<>
struct serde::Serializable<app::dto::external_service_auth::ExternalVerifyTokenRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token", &app::dto::external_service_auth::ExternalVerifyTokenRequest::get_token, &app::dto::external_service_auth::ExternalVerifyTokenRequest::set_token, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "external_verify_token_request"; }
};

template<>
struct serde::Serializable<app::dto::external_service_auth::ExternalSignoutTokenRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token", &app::dto::external_service_auth::ExternalSignoutTokenRequest::get_token, &app::dto::external_service_auth::ExternalSignoutTokenRequest::set_token, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "external_signout_token_request"; }
};

template<>
struct serde::Serializable<app::dto::external_service_auth::ValidateTokenRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token", &app::dto::external_service_auth::ValidateTokenRequest::get_token, &app::dto::external_service_auth::ValidateTokenRequest::set_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"service", &app::dto::external_service_auth::ValidateTokenRequest::get_service, &app::dto::external_service_auth::ValidateTokenRequest::set_service, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "validate_token_request"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::external_service_auth::tests {
using namespace boost::ut;
suite<"app_dto_external_service_auth"> app_dto_external_service_auth_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::external_service_auth::ExternalTokenResponse>::fields())> > 0);
    };
};
}
#endif
