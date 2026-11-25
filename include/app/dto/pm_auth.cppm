module;
export module app_dto:pm_auth;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::pm_auth {

class ExchangeTokenCreateResponse
{
public:
    void set_access_token(std::string value)
    {
        access_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_access_token() const noexcept
    {
        return access_token;
    }

    std::string access_token{};
};

class LinkTokenCreateResponse
{
public:
    void set_link_token(std::string value)
    {
        link_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_link_token() const noexcept
    {
        return link_token;
    }

    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    std::string link_token{};
    std::string connector{};
};

class MerchantConnectorAccountIdValue
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

class PaymentIdValue
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

class PaymentMethodTypeValue
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

class PaymentMethodValue
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

class ExchangeTokenCreateRequest
{
public:
    void set_public_token(std::string value)
    {
        public_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_public_token() const noexcept
    {
        return public_token;
    }

    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(PaymentMethodTypeValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    std::string public_token{};
    std::optional<std::string> client_secret{};
    PaymentIdValue payment_id{};
    PaymentMethodValue payment_method{};
    PaymentMethodTypeValue payment_method_type{};
};

class LinkTokenCreateRequest
{
public:
    void set_language(std::optional<std::string> value)
    {
        language = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_language() const noexcept
    {
        return language;
    }

    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(PaymentMethodTypeValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    std::optional<std::string> language{};
    std::optional<std::string> client_secret{};
    PaymentIdValue payment_id{};
    PaymentMethodValue payment_method{};
    PaymentMethodTypeValue payment_method_type{};
};

class PaymentMethodAuthConnectorChoice
{
public:
    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(PaymentMethodTypeValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_connector_name(std::string value)
    {
        connector_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_name() const noexcept
    {
        return connector_name;
    }

    void set_mca_id(MerchantConnectorAccountIdValue value)
    {
        mca_id = std::move(value);
    }

    [[nodiscard]] const MerchantConnectorAccountIdValue& get_mca_id() const noexcept
    {
        return mca_id;
    }

    PaymentMethodValue payment_method{};
    PaymentMethodTypeValue payment_method_type{};
    std::string connector_name{};
    MerchantConnectorAccountIdValue mca_id{};
};

class PaymentMethodAuthConfig
{
public:
    void set_enabled_payment_methods(std::vector<PaymentMethodAuthConnectorChoice> value)
    {
        enabled_payment_methods = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentMethodAuthConnectorChoice>&
    get_enabled_payment_methods() const noexcept
    {
        return enabled_payment_methods;
    }

    std::vector<PaymentMethodAuthConnectorChoice> enabled_payment_methods{};
};
} // namespace app::dto::pm_auth

template<>
struct serde::Serializable<app::dto::pm_auth::LinkTokenCreateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "language",
                &app::dto::pm_auth::LinkTokenCreateRequest::get_language,
                &app::dto::pm_auth::LinkTokenCreateRequest::set_language,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::dto::pm_auth::LinkTokenCreateRequest::get_client_secret,
                &app::dto::pm_auth::LinkTokenCreateRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::dto::pm_auth::LinkTokenCreateRequest::get_payment_id,
                &app::dto::pm_auth::LinkTokenCreateRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::dto::pm_auth::LinkTokenCreateRequest::get_payment_method,
                &app::dto::pm_auth::LinkTokenCreateRequest::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::dto::pm_auth::LinkTokenCreateRequest::get_payment_method_type,
                &app::dto::pm_auth::LinkTokenCreateRequest::set_payment_method_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "link_token_create_request";
    }
};

template<>
struct serde::Serializable<app::dto::pm_auth::LinkTokenCreateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "link_token",
                &app::dto::pm_auth::LinkTokenCreateResponse::get_link_token,
                &app::dto::pm_auth::LinkTokenCreateResponse::set_link_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::dto::pm_auth::LinkTokenCreateResponse::get_connector,
                &app::dto::pm_auth::LinkTokenCreateResponse::set_connector,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "link_token_create_response";
    }
};

template<>
struct serde::Serializable<app::dto::pm_auth::ExchangeTokenCreateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "public_token",
                &app::dto::pm_auth::ExchangeTokenCreateRequest::get_public_token,
                &app::dto::pm_auth::ExchangeTokenCreateRequest::set_public_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::dto::pm_auth::ExchangeTokenCreateRequest::get_client_secret,
                &app::dto::pm_auth::ExchangeTokenCreateRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::dto::pm_auth::ExchangeTokenCreateRequest::get_payment_id,
                &app::dto::pm_auth::ExchangeTokenCreateRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::dto::pm_auth::ExchangeTokenCreateRequest::get_payment_method,
                &app::dto::pm_auth::ExchangeTokenCreateRequest::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::dto::pm_auth::ExchangeTokenCreateRequest::get_payment_method_type,
                &app::dto::pm_auth::ExchangeTokenCreateRequest::set_payment_method_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "exchange_token_create_request";
    }
};

template<>
struct serde::Serializable<app::dto::pm_auth::ExchangeTokenCreateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "access_token",
            &app::dto::pm_auth::ExchangeTokenCreateResponse::get_access_token,
            &app::dto::pm_auth::ExchangeTokenCreateResponse::set_access_token,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "exchange_token_create_response";
    }
};

template<>
struct serde::Serializable<app::dto::pm_auth::PaymentMethodAuthConnectorChoice>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method",
                &app::dto::pm_auth::PaymentMethodAuthConnectorChoice::get_payment_method,
                &app::dto::pm_auth::PaymentMethodAuthConnectorChoice::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::dto::pm_auth::PaymentMethodAuthConnectorChoice::get_payment_method_type,
                &app::dto::pm_auth::PaymentMethodAuthConnectorChoice::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_name",
                &app::dto::pm_auth::PaymentMethodAuthConnectorChoice::get_connector_name,
                &app::dto::pm_auth::PaymentMethodAuthConnectorChoice::set_connector_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mca_id",
                &app::dto::pm_auth::PaymentMethodAuthConnectorChoice::get_mca_id,
                &app::dto::pm_auth::PaymentMethodAuthConnectorChoice::set_mca_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_auth_connector_choice";
    }
};

template<>
struct serde::Serializable<app::dto::pm_auth::PaymentMethodAuthConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "enabled_payment_methods",
            &app::dto::pm_auth::PaymentMethodAuthConfig::get_enabled_payment_methods,
            &app::dto::pm_auth::PaymentMethodAuthConfig::set_enabled_payment_methods,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_auth_config";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::pm_auth::tests {
using namespace boost::ut;
suite<"app_dto_pm_auth"> app_dto_pm_auth_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::dto::pm_auth::LinkTokenCreateRequest>::fields())> > 0
        );
    };
};
} // namespace app::dto::pm_auth::tests
#endif

// Parity note (pm_auth DTOs): ExchangeTokenCreateResponse above is a real typed DTO mirroring
// api_models/src/pm_auth.rs:33-36 (access_token). Upstream exchange_token_core returns
// RouterResponse<()> / ApplicationResponse::StatusOk (core/pm_auth.rs:244,311), so this type is
// never serialized by either local route; it is retained field-for-field for parity with the
// upstream API model while keeping the module compilable.
