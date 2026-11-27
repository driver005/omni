export module app_auth:hypersense_result;

import std;
import core_serde;

export namespace app::handlers::hypersense {

/**
 * @brief Real typed DTO mirroring ExternalVerifyTokenResponse::Hypersense (api_models:31-39).
 *
 * The shared DTO keeps an empty enum marker (dto/external_service_auth.cppm:36-40), so this
 * handler owns a concrete typed model carrying all four upstream fields (user_id, merchant_id,
 * name, email) with its own serde Serializable specialization below. The untagged upstream
 * enum serializes as bare struct fields, which is exactly what this typed model emits — the
 * empty marker is never the handler's return type (parity report: MATCH, real typed DTO).
 */
class HypersenseVerifyResponse : public serde::Serializer<HypersenseVerifyResponse>
{
public:
    /**
     * @brief Exact wire fields, captured from the retired VerifyTokenResult Serializable
     * specialization in declaration order.
     */
    [[nodiscard]] static constexpr auto wire_fields()
    {
        return std::array<std::string_view, 4>{"user_id", "merchant_id", "name", "email"};
    }

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

/**
 * @brief Exact wire fields of ExternalTokenResponse, captured from the retired GetTokenResult
 * Serializable specialization.
 */
[[nodiscard]] inline constexpr auto get_token_wire_fields()
{
    return std::array<std::string_view, 1>{"token"};
}

// ApplicationResponse::StatusOk (bodyless) needs no model: the runtime returns void and the
// handler writes an empty body directly.

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
