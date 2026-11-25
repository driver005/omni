module;
#include <simdjson.h>
#include <toml++/toml.hpp>
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
    void set_token(std::string value)
    {
        token = std::move(value);
    }

    [[nodiscard]] const std::string& get_token() const noexcept
    {
        return token;
    }

    std::string token{};
};

class ExternalTokenResponse
{
public:
    void set_token(std::string value)
    {
        token = std::move(value);
    }

    [[nodiscard]] const std::string& get_token() const noexcept
    {
        return token;
    }

    std::string token{};
};

class ExternalVerifyTokenRequest
{
public:
    void set_token(std::string value)
    {
        token = std::move(value);
    }

    [[nodiscard]] const std::string& get_token() const noexcept
    {
        return token;
    }

    std::string token{};
};

/**
 * @brief Upstream `ValidatingService` (api_models/external_service_auth.rs:18-23).
 *
 * Upstream derives `#[serde(rename_all = "snake_case")]` and
 * `#[strum(serialize_all = "snake_case")]`, so the one wire spelling is
 * "offer_engine". The serde::FieldConverter below maps the typed enumerator to
 * and from that exact snake_case string, so this DTO itself accepts the upstream
 * wire value (no out-of-band wire normalizer required).
 */
enum class ValidatingService
{
    OfferEngine,
};

/**
 * @brief Typed `ExternalVerifyTokenResponse::OfferEngine` arm.
 *
 * Upstream `ExternalVerifyTokenResponse` is an `#[serde(untagged)]` enum
 * (api_models/external_service_auth.rs:31-46), so its wire form is a bare object
 * with the arm's fields in declaration order and no variant tag. This typed DTO
 * carries the exact four OfferEngine fields (api_models:40-45); it is the return
 * type of this domain's `validate_token` route. `token` is masked upstream because
 * it is a `Secret<String>` (api_models:43).
 */
class ExternalVerifyTokenResponse
{
public:
    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_context(std::string value)
    {
        context = std::move(value);
    }

    [[nodiscard]] const std::string& get_context() const noexcept
    {
        return context;
    }

    void set_token(std::string value)
    {
        token = std::move(value);
    }

    [[nodiscard]] const std::string& get_token() const noexcept
    {
        return token;
    }

    void set_permissions(std::vector<std::string> value)
    {
        permissions = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_permissions() const noexcept
    {
        return permissions;
    }

    std::string merchant_id{};
    std::string context{};
    std::string token{};
    std::vector<std::string> permissions{};
};

/**
 * @brief Typed `ExternalVerifyTokenResponse::Hypersense` arm (api_models:34-39).
 *
 * The other untagged variant, emitted only by the hypersense domain's
 * `verify_external_token` (core/external_service_auth.rs:95-102, wired at
 * routes/hypersense.rs:57-79) and never by this domain's `validate_token`
 * (ValidatingService has only OfferEngine). Both arms are now real typed DTOs.
 */
class ExternalVerifyTokenResponseHypersense
{
public:
    void set_user_id(std::string value)
    {
        user_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_user_id() const noexcept
    {
        return user_id;
    }

    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_name(std::string value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    void set_email(std::string value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::string& get_email() const noexcept
    {
        return email;
    }

    std::string user_id{};
    std::string merchant_id{};
    std::string name{};
    std::string email{};
};

class ValidateTokenRequest
{
public:
    void set_token(std::string value)
    {
        token = std::move(value);
    }

    [[nodiscard]] const std::string& get_token() const noexcept
    {
        return token;
    }

    void set_service(ValidatingService value)
    {
        service = std::move(value);
    }

    [[nodiscard]] const ValidatingService& get_service() const noexcept
    {
        return service;
    }

    std::string token{};
    ValidatingService service{};
};
} // namespace app::dto::external_service_auth

// ── Exact upstream wire serde ─────────────────────────────────────────────────
// ValidatingService is `#[serde(rename_all = "snake_case")]` + strum snake_case
// (api_models:18-23): the typed enum round-trips as the string "offer_engine".

template<>
struct serde::FieldConverter<app::dto::external_service_auth::ValidatingService>
{
    using Enum = app::dto::external_service_auth::ValidatingService;
    using rfl_type = std::string;

    [[nodiscard]] static rfl_type to_rfl(const Enum& value)
    {
        switch (value) {
            case Enum::OfferEngine:
                return "offer_engine";
        }
        return "offer_engine";
    }

    [[nodiscard]] static Enum from_rfl(const rfl_type& value)
    {
        if (value == "offer_engine") {
            return Enum::OfferEngine;
        }
        return Enum::OfferEngine;
    }

    static simdjson::error_code from_simdjson(simdjson::ondemand::value& json_value, Enum& out)
    {
        std::string_view string_value;
        if (auto ec = json_value.get_string().get(string_value); ec) {
            return ec;
        }
        if (string_value == "offer_engine") {
            out = Enum::OfferEngine;
            return simdjson::SUCCESS;
        }
        return simdjson::INCORRECT_TYPE;
    }

    [[nodiscard]] static std::expected<Enum, std::string>
    from_toml(const toml::table& table, std::string_view field_name)
    {
        const auto string_value = table[field_name].value<std::string>();
        if (!string_value) {
            return std::unexpected{std::format("missing field '{}'", field_name)};
        }
        if (*string_value == "offer_engine") {
            return Enum::OfferEngine;
        }
        return std::unexpected{
            std::format("invalid ValidatingService '{}' for field '{}'", *string_value, field_name)
        };
    }
};

template<>
struct serde::Serializable<app::dto::external_service_auth::ExternalTokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "token",
            &app::dto::external_service_auth::ExternalTokenResponse::get_token,
            &app::dto::external_service_auth::ExternalTokenResponse::set_token,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "external_token_response";
    }
};

template<>
struct serde::Serializable<app::dto::external_service_auth::ExternalVerifyTokenRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "token",
            &app::dto::external_service_auth::ExternalVerifyTokenRequest::get_token,
            &app::dto::external_service_auth::ExternalVerifyTokenRequest::set_token,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "external_verify_token_request";
    }
};

template<>
struct serde::Serializable<app::dto::external_service_auth::ExternalSignoutTokenRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "token",
            &app::dto::external_service_auth::ExternalSignoutTokenRequest::get_token,
            &app::dto::external_service_auth::ExternalSignoutTokenRequest::set_token,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "external_signout_token_request";
    }
};

template<>
struct serde::Serializable<app::dto::external_service_auth::ValidateTokenRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "token",
                &app::dto::external_service_auth::ValidateTokenRequest::get_token,
                &app::dto::external_service_auth::ValidateTokenRequest::set_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "service",
                &app::dto::external_service_auth::ValidateTokenRequest::get_service,
                &app::dto::external_service_auth::ValidateTokenRequest::set_service,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "validate_token_request";
    }
};

template<>
struct serde::Serializable<app::dto::external_service_auth::ExternalVerifyTokenResponse>
{
    static constexpr auto fields()
    {
        using R = app::dto::external_service_auth::ExternalVerifyTokenResponse;
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &R::get_merchant_id,
                &R::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"context", &R::get_context, &R::set_context, serde::FieldOptions{}>{},
            serde::FieldDesc<"token", &R::get_token, &R::set_token, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "permissions",
                &R::get_permissions,
                &R::set_permissions,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "external_verify_token_response";
    }
};

template<>
struct serde::Serializable<app::dto::external_service_auth::ExternalVerifyTokenResponseHypersense>
{
    static constexpr auto fields()
    {
        using R = app::dto::external_service_auth::ExternalVerifyTokenResponseHypersense;
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
        return "external_verify_token_response_hypersense";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::external_service_auth::tests {
using namespace boost::ut;
suite<"app_dto_external_service_auth"> app_dto_external_service_auth_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::dto::external_service_auth::ExternalTokenResponse>::fields())> > 0
        );
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::dto::external_service_auth::
                                                 ExternalVerifyTokenResponse>::fields())> == 4
        );
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::dto::external_service_auth::
                                           ExternalVerifyTokenResponseHypersense>::fields())> == 4
        );
    };
    "ValidatingService snake_case serde matches upstream"_test = []
    {
        using Enum = app::dto::external_service_auth::ValidatingService;
        using Converter = serde::FieldConverter<Enum>;
        expect(Converter::to_rfl(Enum::OfferEngine) == "offer_engine");
        expect(Converter::from_rfl("offer_engine") == Enum::OfferEngine);
        expect(Converter::from_toml(toml::table{}, "service").has_value() == false);
        const auto table = toml::parse(R"(service = "offer_engine")");
        const auto parsed = Converter::from_toml(table, "service");
        expect(parsed.has_value());
        expect(parsed.value() == Enum::OfferEngine);
    };
    "OfferEngine response emits upstream untagged field order"_test = []
    {
        using Response = app::dto::external_service_auth::ExternalVerifyTokenResponse;
        Response response{};
        response.set_merchant_id("merchant_1");
        response.set_context("MERCHANT");
        response.set_token("masked");
        response.set_permissions({"offers_read", "offers_write"});
        const auto json = serde::Ser::encode_json(response);
        expect(json.find(R"("merchant_id":"merchant_1")") != std::string::npos);
        expect(json.find(R"("context":"MERCHANT")") != std::string::npos);
        expect(json.find(R"("token":"masked")") != std::string::npos);
        expect(json.find(R"("permissions":["offers_read","offers_write"])") != std::string::npos);
    };
};
} // namespace app::dto::external_service_auth::tests
#endif
