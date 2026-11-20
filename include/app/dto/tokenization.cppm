module;
export module app_dto:tokenization;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::tokenization {

/**
 * @brief Route wire contract (api_models/src/tokenization.rs:8-46).
 *
 * Upstream id newtypes (`GlobalTokenId`, `GlobalCustomerId`,
 * `GlobalPaymentMethodSessionId`), `time::PrimitiveDateTime` and
 * `common_enums::TokenizationFlag` all serialize as plain JSON scalars. The four
 * route DTOs below therefore store those values as plain `std::string` fields so the
 * shared `serde`/rfl reflector emits the exact upstream wire shape:
 *   GenericTokenizationResponse -> {"id":...,"created_at":...,"flag":...}
 *   DeleteTokenDataResponse     -> {"id":...}
 * This mirrors the typed-DTO pattern already used by `app::dto::client_secret`.
 * The `*Value` newtypes further down are retained for other consumers; they are not
 * the route wire surface (their `value` field would emit a `{"value":...}` wrapper).
 */

class GlobalCustomerIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class GlobalPaymentMethodSessionIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class GlobalTokenIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PrimitiveDateTimeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class TokenizationFlagValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class DeleteTokenDataRequest
{
public:
    // Upstream: customer_id/session_id are id newtypes -> plain strings on the wire.
    void set_customer_id(std::string value) { customer_id = std::move(value); }
    [[nodiscard]] const std::string& get_customer_id() const noexcept { return customer_id; }
    void set_session_id(std::string value) { session_id = std::move(value); }
    [[nodiscard]] const std::string& get_session_id() const noexcept { return session_id; }
    std::string customer_id{};
    std::string session_id{};
};

class DeleteTokenDataResponse
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    std::string id{};
};

class GenericTokenizationRequest
{
public:
    void set_customer_id(std::string value) { customer_id = std::move(value); }
    [[nodiscard]] const std::string& get_customer_id() const noexcept { return customer_id; }
    // Secret<serde_json::Value> is transparent on the wire: opaque JSON text.
    void set_token_request(std::string value) { token_request = std::move(value); }
    [[nodiscard]] const std::string& get_token_request() const noexcept { return token_request; }
    std::string customer_id{};
    std::string token_request{};
};

class GenericTokenizationResponse
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_created_at(std::string value) { created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return created_at; }
    void set_flag(std::string value) { flag = std::move(value); }
    [[nodiscard]] const std::string& get_flag() const noexcept { return flag; }
    std::string id{};
    std::string created_at{};
    std::string flag{};
};
}

template<>
struct serde::Serializable<app::dto::tokenization::GlobalTokenIdValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::tokenization::GlobalTokenIdValue::get_value, &app::dto::tokenization::GlobalTokenIdValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "global_token_id_value"; }
};

template<>
struct serde::Serializable<app::dto::tokenization::PrimitiveDateTimeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::tokenization::PrimitiveDateTimeValue::get_value, &app::dto::tokenization::PrimitiveDateTimeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "primitive_date_time_value"; }
};

template<>
struct serde::Serializable<app::dto::tokenization::TokenizationFlagValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::tokenization::TokenizationFlagValue::get_value, &app::dto::tokenization::TokenizationFlagValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "tokenization_flag_value"; }
};

template<>
struct serde::Serializable<app::dto::tokenization::GlobalCustomerIdValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::tokenization::GlobalCustomerIdValue::get_value, &app::dto::tokenization::GlobalCustomerIdValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "global_customer_id_value"; }
};

template<>
struct serde::Serializable<app::dto::tokenization::GlobalPaymentMethodSessionIdValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::tokenization::GlobalPaymentMethodSessionIdValue::get_value, &app::dto::tokenization::GlobalPaymentMethodSessionIdValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "global_payment_method_session_id_value"; }
};

template<>
struct serde::Serializable<app::dto::tokenization::GenericTokenizationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::dto::tokenization::GenericTokenizationResponse::get_id, &app::dto::tokenization::GenericTokenizationResponse::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::dto::tokenization::GenericTokenizationResponse::get_created_at, &app::dto::tokenization::GenericTokenizationResponse::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"flag", &app::dto::tokenization::GenericTokenizationResponse::get_flag, &app::dto::tokenization::GenericTokenizationResponse::set_flag, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "generic_tokenization_response"; }
};

template<>
struct serde::Serializable<app::dto::tokenization::GenericTokenizationRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"customer_id", &app::dto::tokenization::GenericTokenizationRequest::get_customer_id, &app::dto::tokenization::GenericTokenizationRequest::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"token_request", &app::dto::tokenization::GenericTokenizationRequest::get_token_request, &app::dto::tokenization::GenericTokenizationRequest::set_token_request, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "generic_tokenization_request"; }
};

template<>
struct serde::Serializable<app::dto::tokenization::DeleteTokenDataRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"customer_id", &app::dto::tokenization::DeleteTokenDataRequest::get_customer_id, &app::dto::tokenization::DeleteTokenDataRequest::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"session_id", &app::dto::tokenization::DeleteTokenDataRequest::get_session_id, &app::dto::tokenization::DeleteTokenDataRequest::set_session_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "delete_token_data_request"; }
};

template<>
struct serde::Serializable<app::dto::tokenization::DeleteTokenDataResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::dto::tokenization::DeleteTokenDataResponse::get_id, &app::dto::tokenization::DeleteTokenDataResponse::set_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "delete_token_data_response"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::tokenization::tests {
using namespace boost::ut;
suite<"app_dto_tokenization"> app_dto_tokenization_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::tokenization::GenericTokenizationResponse>::fields())> == 3);
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::tokenization::DeleteTokenDataResponse>::fields())> == 1);
    };
    "route DTOs are the transparent wire surface"_test = [] {
        GenericTokenizationResponse response{};
        response.set_id("0a_tok_01926c58bc6e77c09e809964e72af8c8");
        response.set_created_at("2024-02-24 11:04:09.922000000");
        response.set_flag("enabled");
        const auto json = serde::Ser::encode_json(response);
        expect(json.find(R"("id":"0a_tok_)") != std::string::npos);
        expect(json.find(R"("flag":"enabled")") != std::string::npos);
        expect(json.find("value") == std::string::npos); // no {"value":...} wrapper
    };
};
}
#endif
