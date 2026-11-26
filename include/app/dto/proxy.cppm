module;
export module app_dto:proxy;

import core_serde;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::proxy {

class Headers : public serde::Serializer<Headers>
{
public:
    void set_value(std::unordered_map<std::string, std::string> v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::unordered_map<std::string, std::string>& get_value() const noexcept
    {
        return value;
    }

    std::unordered_map<std::string, std::string> value{};
};

class MethodValue : public serde::Serializer<MethodValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class ProxyResponseBody
{
    Json,
    Text,
};

class SafeOutboundUrlValue : public serde::Serializer<SafeOutboundUrlValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class TokenType
{
    TokenizationId,
    PaymentMethodId,
    VolatilePaymentMethodId,
    PaymentMethodToken,
};

class ProxyRequest : public serde::Serializer<ProxyRequest>
{
public:
    void set_request_body(serde::Value value)
    {
        request_body = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_request_body() const noexcept
    {
        return request_body;
    }

    void set_destination_url(SafeOutboundUrlValue value)
    {
        destination_url = std::move(value);
    }

    [[nodiscard]] const SafeOutboundUrlValue& get_destination_url() const noexcept
    {
        return destination_url;
    }

    void set_headers(Headers value)
    {
        headers = std::move(value);
    }

    [[nodiscard]] const Headers& get_headers() const noexcept
    {
        return headers;
    }

    void set_method(MethodValue value)
    {
        method = std::move(value);
    }

    [[nodiscard]] const MethodValue& get_method() const noexcept
    {
        return method;
    }

    void set_token(std::string value)
    {
        token = std::move(value);
    }

    [[nodiscard]] const std::string& get_token() const noexcept
    {
        return token;
    }

    void set_token_type(TokenType value)
    {
        token_type = std::move(value);
    }

    [[nodiscard]] const TokenType& get_token_type() const noexcept
    {
        return token_type;
    }

    serde::Value request_body{};
    SafeOutboundUrlValue destination_url{};
    Headers headers{};
    MethodValue method{};
    std::string token{};
    TokenType token_type{};
};

class ProxyResponse : public serde::Serializer<ProxyResponse>
{
public:
    void set_response(ProxyResponseBody value)
    {
        response = std::move(value);
    }

    [[nodiscard]] const ProxyResponseBody& get_response() const noexcept
    {
        return response;
    }

    void set_status_code(std::uint16_t value)
    {
        status_code = std::move(value);
    }

    [[nodiscard]] const std::uint16_t& get_status_code() const noexcept
    {
        return status_code;
    }

    void set_response_headers(Headers value)
    {
        response_headers = std::move(value);
    }

    [[nodiscard]] const Headers& get_response_headers() const noexcept
    {
        return response_headers;
    }

    ProxyResponseBody response{};
    std::uint16_t status_code{};
    Headers response_headers{};
};
} // namespace app::dto::proxy

template<>
struct serde::Serializable<app::dto::proxy::Headers>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::dto::proxy::Headers::get_value,
            &app::dto::proxy::Headers::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "headers";
    }
};

template<>
struct serde::Serializable<app::dto::proxy::ProxyRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "request_body",
                &app::dto::proxy::ProxyRequest::get_request_body,
                &app::dto::proxy::ProxyRequest::set_request_body,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "destination_url",
                &app::dto::proxy::ProxyRequest::get_destination_url,
                &app::dto::proxy::ProxyRequest::set_destination_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "headers",
                &app::dto::proxy::ProxyRequest::get_headers,
                &app::dto::proxy::ProxyRequest::set_headers,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "method",
                &app::dto::proxy::ProxyRequest::get_method,
                &app::dto::proxy::ProxyRequest::set_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token",
                &app::dto::proxy::ProxyRequest::get_token,
                &app::dto::proxy::ProxyRequest::set_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_type",
                &app::dto::proxy::ProxyRequest::get_token_type,
                &app::dto::proxy::ProxyRequest::set_token_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "proxy_request";
    }
};

template<>
struct serde::Serializable<app::dto::proxy::ProxyResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "response",
                &app::dto::proxy::ProxyResponse::get_response,
                &app::dto::proxy::ProxyResponse::set_response,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status_code",
                &app::dto::proxy::ProxyResponse::get_status_code,
                &app::dto::proxy::ProxyResponse::set_status_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "response_headers",
                &app::dto::proxy::ProxyResponse::get_response_headers,
                &app::dto::proxy::ProxyResponse::set_response_headers,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "proxy_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::proxy::tests {
using namespace boost::ut;
suite<"app_dto_proxy"> app_dto_proxy_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<app::dto::proxy::Headers>::fields())> > 0
        );
    };
};
} // namespace app::dto::proxy::tests
#endif
