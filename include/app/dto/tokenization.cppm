module;
export module app_dto:tokenization;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::tokenization {

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
    void set_customer_id(GlobalCustomerIdValue value) { customer_id = std::move(value); }
    [[nodiscard]] const GlobalCustomerIdValue& get_customer_id() const noexcept { return customer_id; }
    void set_session_id(GlobalPaymentMethodSessionIdValue value) { session_id = std::move(value); }
    [[nodiscard]] const GlobalPaymentMethodSessionIdValue& get_session_id() const noexcept { return session_id; }
    GlobalCustomerIdValue customer_id{};
    GlobalPaymentMethodSessionIdValue session_id{};
};

class DeleteTokenDataResponse
{
public:
    void set_id(GlobalTokenIdValue value) { id = std::move(value); }
    [[nodiscard]] const GlobalTokenIdValue& get_id() const noexcept { return id; }
    GlobalTokenIdValue id{};
};

class GenericTokenizationRequest
{
public:
    void set_customer_id(GlobalCustomerIdValue value) { customer_id = std::move(value); }
    [[nodiscard]] const GlobalCustomerIdValue& get_customer_id() const noexcept { return customer_id; }
    void set_token_request(std::string value) { token_request = std::move(value); }
    [[nodiscard]] const std::string& get_token_request() const noexcept { return token_request; }
    GlobalCustomerIdValue customer_id{};
    std::string token_request{};
};

class GenericTokenizationResponse
{
public:
    void set_id(GlobalTokenIdValue value) { id = std::move(value); }
    [[nodiscard]] const GlobalTokenIdValue& get_id() const noexcept { return id; }
    void set_created_at(PrimitiveDateTimeValue value) { created_at = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept { return created_at; }
    void set_flag(TokenizationFlagValue value) { flag = std::move(value); }
    [[nodiscard]] const TokenizationFlagValue& get_flag() const noexcept { return flag; }
    GlobalTokenIdValue id{};
    PrimitiveDateTimeValue created_at{};
    TokenizationFlagValue flag{};
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
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::tokenization::GenericTokenizationResponse>::fields())> > 0);
    };
};
}
#endif
