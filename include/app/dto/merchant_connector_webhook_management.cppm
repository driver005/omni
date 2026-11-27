module;
export module app_dto:merchant_connector_webhook_management;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::merchant_connector_webhook_management {

class ConnectorWebhookEventTypeValue :
    public core::behavior::Serializer<ConnectorWebhookEventTypeValue>
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

enum class ConnectorWebhookScope
{
    NotSpecific,
    PaymentMethodType,
    EventType,
};

enum class Scope
{
    NotSpecific,
    PaymentMethodTypes,
    EventTypes,
};

enum class ScopeIdentifier
{
    NotSpecific,
    PaymentMethodType,
    EventType,
};

enum class ScopeType
{
    NotSpecific,
    PaymentMethodType,
    EventType,
};

class WebhookRegistrationError : public core::behavior::Serializer<WebhookRegistrationError>
{
public:
    void set_code(std::string value)
    {
        code = std::move(value);
    }

    [[nodiscard]] const std::string& get_code() const noexcept
    {
        return code;
    }

    void set_message(std::string value)
    {
        message = std::move(value);
    }

    [[nodiscard]] const std::string& get_message() const noexcept
    {
        return message;
    }

    std::string code{};
    std::string message{};
};

class WebhookRegistrationStatusValue :
    public core::behavior::Serializer<WebhookRegistrationStatusValue>
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

class WebhookSecretErrorDetails : public core::behavior::Serializer<WebhookSecretErrorDetails>
{
public:
    void set_code(std::optional<std::string> value)
    {
        code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_code() const noexcept
    {
        return code;
    }

    void set_message(std::optional<std::string> value)
    {
        message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_message() const noexcept
    {
        return message;
    }

    std::optional<std::string> code{};
    std::optional<std::string> message{};
};

class WebhookSecretGenerationStatusValue :
    public core::behavior::Serializer<WebhookSecretGenerationStatusValue>
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

class ConnectorWebhookRegisterRequest :
    public core::behavior::Serializer<ConnectorWebhookRegisterRequest>
{
public:
    void set_scope(std::optional<Scope> value)
    {
        scope = std::move(value);
    }

    [[nodiscard]] const std::optional<Scope>& get_scope() const noexcept
    {
        return scope;
    }

    void set_event_type(std::optional<ConnectorWebhookEventTypeValue> value)
    {
        event_type = std::move(value);
    }

    [[nodiscard]] const std::optional<ConnectorWebhookEventTypeValue>&
    get_event_type() const noexcept
    {
        return event_type;
    }

    std::optional<Scope> scope{};
    std::optional<ConnectorWebhookEventTypeValue> event_type{};
};

class ConnectorWebhookResponse : public core::behavior::Serializer<ConnectorWebhookResponse>
{
public:
    void set_connector_webhook_id(std::string value)
    {
        connector_webhook_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_webhook_id() const noexcept
    {
        return connector_webhook_id;
    }

    void set_event_type(std::optional<ConnectorWebhookEventTypeValue> value)
    {
        event_type = std::move(value);
    }

    [[nodiscard]] const std::optional<ConnectorWebhookEventTypeValue>&
    get_event_type() const noexcept
    {
        return event_type;
    }

    void set_scope(std::optional<ConnectorWebhookScope> value)
    {
        scope = std::move(value);
    }

    [[nodiscard]] const std::optional<ConnectorWebhookScope>& get_scope() const noexcept
    {
        return scope;
    }

    std::string connector_webhook_id{};
    std::optional<ConnectorWebhookEventTypeValue> event_type{};
    std::optional<ConnectorWebhookScope> scope{};
};

class WebhookRegistrationResult : public core::behavior::Serializer<WebhookRegistrationResult>
{
public:
    void set_identifier(ScopeIdentifier value)
    {
        identifier = std::move(value);
    }

    [[nodiscard]] const ScopeIdentifier& get_identifier() const noexcept
    {
        return identifier;
    }

    void set_status(WebhookRegistrationStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const WebhookRegistrationStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_connector_webhook_id(std::optional<std::string> value)
    {
        connector_webhook_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_webhook_id() const noexcept
    {
        return connector_webhook_id;
    }

    void set_error(std::optional<WebhookRegistrationError> value)
    {
        error = std::move(value);
    }

    [[nodiscard]] const std::optional<WebhookRegistrationError>& get_error() const noexcept
    {
        return error;
    }

    ScopeIdentifier identifier{};
    WebhookRegistrationStatusValue status{};
    std::optional<std::string> connector_webhook_id{};
    std::optional<WebhookRegistrationError> error{};
};

class ConnectorWebhookListResponse : public core::behavior::Serializer<ConnectorWebhookListResponse>
{
public:
    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    void set_webhooks(std::vector<ConnectorWebhookResponse> value)
    {
        webhooks = std::move(value);
    }

    [[nodiscard]] const std::vector<ConnectorWebhookResponse>& get_webhooks() const noexcept
    {
        return webhooks;
    }

    std::string connector{};
    std::vector<ConnectorWebhookResponse> webhooks{};
};

class RegisterConnectorWebhookResponse :
    public core::behavior::Serializer<RegisterConnectorWebhookResponse>
{
public:
    void set_event_type(std::optional<ConnectorWebhookEventTypeValue> value)
    {
        event_type = std::move(value);
    }

    [[nodiscard]] const std::optional<ConnectorWebhookEventTypeValue>&
    get_event_type() const noexcept
    {
        return event_type;
    }

    void set_connector_webhook_id(std::optional<std::string> value)
    {
        connector_webhook_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_webhook_id() const noexcept
    {
        return connector_webhook_id;
    }

    void set_webhook_registration_status(std::optional<WebhookRegistrationStatusValue> value)
    {
        webhook_registration_status = std::move(value);
    }

    [[nodiscard]] const std::optional<WebhookRegistrationStatusValue>&
    get_webhook_registration_status() const noexcept
    {
        return webhook_registration_status;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_secret_generation_status(std::optional<WebhookSecretGenerationStatusValue> value)
    {
        secret_generation_status = std::move(value);
    }

    [[nodiscard]] const std::optional<WebhookSecretGenerationStatusValue>&
    get_secret_generation_status() const noexcept
    {
        return secret_generation_status;
    }

    void set_secret_error(std::optional<WebhookSecretErrorDetails> value)
    {
        secret_error = std::move(value);
    }

    [[nodiscard]] const std::optional<WebhookSecretErrorDetails>& get_secret_error() const noexcept
    {
        return secret_error;
    }

    void set_scope_type(std::optional<ScopeType> value)
    {
        scope_type = std::move(value);
    }

    [[nodiscard]] const std::optional<ScopeType>& get_scope_type() const noexcept
    {
        return scope_type;
    }

    void set_requested(std::optional<std::vector<ScopeIdentifier>> value)
    {
        requested = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<ScopeIdentifier>>& get_requested() const noexcept
    {
        return requested;
    }

    void set_results(std::optional<std::vector<WebhookRegistrationResult>> value)
    {
        results = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<WebhookRegistrationResult>>&
    get_results() const noexcept
    {
        return results;
    }

    std::optional<ConnectorWebhookEventTypeValue> event_type{};
    std::optional<std::string> connector_webhook_id{};
    std::optional<WebhookRegistrationStatusValue> webhook_registration_status{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::optional<WebhookSecretGenerationStatusValue> secret_generation_status{};
    std::optional<WebhookSecretErrorDetails> secret_error{};
    std::optional<ScopeType> scope_type{};
    std::optional<std::vector<ScopeIdentifier>> requested{};
    std::optional<std::vector<WebhookRegistrationResult>> results{};
};
} // namespace app::dto::merchant_connector_webhook_management

template<>
struct serde::Serializable<
    app::dto::merchant_connector_webhook_management::ConnectorWebhookEventTypeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::dto::merchant_connector_webhook_management::ConnectorWebhookEventTypeValue::
                get_value,
            &app::dto::merchant_connector_webhook_management::ConnectorWebhookEventTypeValue::
                set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "connector_webhook_event_type_value";
    }
};

template<>
struct serde::Serializable<
    app::dto::merchant_connector_webhook_management::WebhookRegistrationStatusValue>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::dto::merchant_connector_webhook_management::WebhookRegistrationStatusValue::
                get_value,
            &app::dto::merchant_connector_webhook_management::WebhookRegistrationStatusValue::
                set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "webhook_registration_status_value";
    }
};

template<>
struct serde::Serializable<
    app::dto::merchant_connector_webhook_management::WebhookSecretGenerationStatusValue>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::dto::merchant_connector_webhook_management::WebhookSecretGenerationStatusValue::
                get_value,
            &app::dto::merchant_connector_webhook_management::WebhookSecretGenerationStatusValue::
                set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "webhook_secret_generation_status_value";
    }
};

template<>
struct serde::Serializable<
    app::dto::merchant_connector_webhook_management::WebhookRegistrationError>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "code",
                &app::dto::merchant_connector_webhook_management::WebhookRegistrationError::
                    get_code,
                &app::dto::merchant_connector_webhook_management::WebhookRegistrationError::
                    set_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message",
                &app::dto::merchant_connector_webhook_management::WebhookRegistrationError::
                    get_message,
                &app::dto::merchant_connector_webhook_management::WebhookRegistrationError::
                    set_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "webhook_registration_error";
    }
};

template<>
struct serde::Serializable<
    app::dto::merchant_connector_webhook_management::WebhookRegistrationResult>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "identifier",
                &app::dto::merchant_connector_webhook_management::WebhookRegistrationResult::
                    get_identifier,
                &app::dto::merchant_connector_webhook_management::WebhookRegistrationResult::
                    set_identifier,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::dto::merchant_connector_webhook_management::WebhookRegistrationResult::
                    get_status,
                &app::dto::merchant_connector_webhook_management::WebhookRegistrationResult::
                    set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_webhook_id",
                &app::dto::merchant_connector_webhook_management::WebhookRegistrationResult::
                    get_connector_webhook_id,
                &app::dto::merchant_connector_webhook_management::WebhookRegistrationResult::
                    set_connector_webhook_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error",
                &app::dto::merchant_connector_webhook_management::WebhookRegistrationResult::
                    get_error,
                &app::dto::merchant_connector_webhook_management::WebhookRegistrationResult::
                    set_error,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "webhook_registration_result";
    }
};

template<>
struct serde::Serializable<
    app::dto::merchant_connector_webhook_management::ConnectorWebhookRegisterRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "scope",
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookRegisterRequest::
                    get_scope,
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookRegisterRequest::
                    set_scope,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_type",
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookRegisterRequest::
                    get_event_type,
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookRegisterRequest::
                    set_event_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "connector_webhook_register_request";
    }
};

template<>
struct serde::Serializable<
    app::dto::merchant_connector_webhook_management::WebhookSecretErrorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "code",
                &app::dto::merchant_connector_webhook_management::WebhookSecretErrorDetails::
                    get_code,
                &app::dto::merchant_connector_webhook_management::WebhookSecretErrorDetails::
                    set_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message",
                &app::dto::merchant_connector_webhook_management::WebhookSecretErrorDetails::
                    get_message,
                &app::dto::merchant_connector_webhook_management::WebhookSecretErrorDetails::
                    set_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "webhook_secret_error_details";
    }
};

template<>
struct serde::Serializable<
    app::dto::merchant_connector_webhook_management::ConnectorWebhookResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector_webhook_id",
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookResponse::
                    get_connector_webhook_id,
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookResponse::
                    set_connector_webhook_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_type",
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookResponse::
                    get_event_type,
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookResponse::
                    set_event_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scope",
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookResponse::
                    get_scope,
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookResponse::
                    set_scope,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "connector_webhook_response";
    }
};

template<>
struct serde::Serializable<
    app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "event_type",
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    get_event_type,
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    set_event_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_webhook_id",
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    get_connector_webhook_id,
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    set_connector_webhook_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "webhook_registration_status",
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    get_webhook_registration_status,
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    set_webhook_registration_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    get_error_code,
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    get_error_message,
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "secret_generation_status",
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    get_secret_generation_status,
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    set_secret_generation_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "secret_error",
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    get_secret_error,
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    set_secret_error,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scope_type",
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    get_scope_type,
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    set_scope_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "requested",
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    get_requested,
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    set_requested,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "results",
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    get_results,
                &app::dto::merchant_connector_webhook_management::RegisterConnectorWebhookResponse::
                    set_results,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "register_connector_webhook_response";
    }
};

template<>
struct serde::Serializable<
    app::dto::merchant_connector_webhook_management::ConnectorWebhookListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookListResponse::
                    get_connector,
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookListResponse::
                    set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "webhooks",
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookListResponse::
                    get_webhooks,
                &app::dto::merchant_connector_webhook_management::ConnectorWebhookListResponse::
                    set_webhooks,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "connector_webhook_list_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::merchant_connector_webhook_management::tests {
using namespace boost::ut;
suite<"app_dto_merchant_connector_webhook_management">
    app_dto_merchant_connector_webhook_management_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::dto::merchant_connector_webhook_management::
                                                 WebhookRegistrationResult>::fields())> > 0
        );
    };
};
} // namespace app::dto::merchant_connector_webhook_management::tests
#endif
