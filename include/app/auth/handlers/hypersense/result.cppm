export module app_auth:hypersense_result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::hypersense {

/**
 * @brief Real typed DTO mirroring ExternalVerifyTokenResponse::Hypersense (api_models:31-39).
 *
 * The shared DTO keeps an empty enum marker (dto/external_service_auth.cppm:36-40), so this
 * handler owns a concrete typed DTO carrying all four upstream fields (user_id, merchant_id,
 * name, email) with its own serde Serializable specialization below. The untagged upstream
 * enum serializes as bare struct fields, which is exactly what this typed DTO emits — the
 * empty marker is never the handler's return type (parity report: MATCH, real typed DTO).
 */
class HypersenseVerifyResponse
{
public:
    void set_user_id(std::string value)
    {
        m_user_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_user_id() const noexcept
    {
        return m_user_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_name(std::string value)
    {
        m_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    void set_email(std::string value)
    {
        m_email = std::move(value);
    }

    [[nodiscard]] const std::string& get_email() const noexcept
    {
        return m_email;
    }

    std::string m_user_id{};
    std::string m_merchant_id{};
    std::string m_name{};
    std::string m_email{};
};

// The two JSON-shaped responses inherit their wire fields from the exact typed DTO
// the old core encoded, so field names, order and wrapper types stay byte-identical.
// The Serializable specializations reuse the DTO FieldDesc tuples; those descriptors
// apply to the derived object just as they do to the base object.
class GetTokenResult : public ::app::dto::external_service_auth::ExternalTokenResponse
{
public:
    GetTokenResult() = default;

    explicit GetTokenResult(::app::dto::external_service_auth::ExternalTokenResponse value) noexcept
        :
        ExternalTokenResponse{std::move(value)}
    {
    }
};

class VerifyTokenResult : public HypersenseVerifyResponse
{
public:
    VerifyTokenResult() = default;

    explicit VerifyTokenResult(HypersenseVerifyResponse value) noexcept :
        HypersenseVerifyResponse{std::move(value)}
    {
    }
};

// ApplicationResponse::StatusOk (bodyless). The handler writes status OK and an
// empty body directly; there is no JSON payload to serialize.
struct SignoutTokenResult
{};

} // namespace app::handlers::hypersense

template<>
struct serde::Serializable<::app::handlers::hypersense::HypersenseVerifyResponse>
{
    static constexpr auto fields()
    {
        using R = ::app::handlers::hypersense::HypersenseVerifyResponse;
        return std::tuple{
            serde::FieldDesc<"user_id", &R::get_user_id, &R::set_user_id, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &R::get_merchant_id,
                &R::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &R::get_name, &R::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &R::get_email, &R::set_email, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "hypersense_verify_response";
    }
};

template<>
struct serde::Serializable<::app::handlers::hypersense::GetTokenResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            ::app::dto::external_service_auth::ExternalTokenResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_token_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::hypersense::VerifyTokenResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::handlers::hypersense::HypersenseVerifyResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "verify_token_result";
    }
};
