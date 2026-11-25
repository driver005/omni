module;
export module app_dto:webhooks;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::webhooks {

enum class AuthenticationIdType
{
    AuthenticationId,
    ConnectorAuthenticationId,
};

class ConnectorWebhookSecrets
{
public:
    void set_secret(std::vector<std::uint8_t> value)
    {
        secret = std::move(value);
    }

    [[nodiscard]] const std::vector<std::uint8_t>& get_secret() const noexcept
    {
        return secret;
    }

    void set_additional_secret(std::optional<std::string> value)
    {
        additional_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_additional_secret() const noexcept
    {
        return additional_secret;
    }

    std::vector<std::uint8_t> secret{};
    std::optional<std::string> additional_secret{};
};

class EventTypeValue
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

enum class IncomingWebhookEvent
{
    PaymentIntentFailure,
    PaymentIntentSuccess,
    PaymentIntentProcessing,
    PaymentIntentPartiallyFunded,
    PaymentIntentCancelled,
    PaymentIntentCancelFailure,
    PaymentIntentAuthorizationSuccess,
    PaymentIntentAuthorizationFailure,
    PaymentIntentExtendAuthorizationSuccess,
    PaymentIntentExtendAuthorizationFailure,
    PaymentIntentCaptureSuccess,
    PaymentIntentCaptureFailure,
    PaymentIntentExpired,
    PaymentActionRequired,
    PaymentAssociatedDataUpdate,
    EventNotSupported,
    SourceChargeable,
    SourceTransactionCreated,
    RefundFailure,
    RefundSuccess,
    RefundReview,
    DisputeOpened,
    DisputeExpired,
    DisputeAccepted,
    DisputeCancelled,
    DisputeChallenged,
    DisputeWon,
    DisputeLost,
    MandateActive,
    MandateRevoked,
    MandateActionRequired,
    EndpointVerification,
    ExternalAuthenticationARes,
    FrmApproved,
    FrmRejected,
    PayoutSuccess,
    PayoutFailure,
    PayoutProcessing,
    PayoutCancelled,
    PayoutCreated,
    PayoutExpired,
    PayoutReversed,
    RecoveryPaymentFailure,
    RecoveryPaymentSuccess,
    RecoveryPaymentPending,
    RecoveryInvoiceCancel,
    SetupWebhook,
    InvoiceGenerated,
};

enum class InvoiceIdType
{
    ConnectorInvoiceId,
};

enum class MandateIdType
{
    MandateId,
    ConnectorMandateId,
};

class MerchantIdValue
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

enum class ObjectReferenceId
{
    PaymentId,
    RefundId,
    MandateId,
    ExternalAuthenticationID,
    PayoutId,
    InvoiceId,
    SubscriptionId,
};

enum class OutgoingWebhookContent
{
    PaymentDetails,
    RefundDetails,
    DisputeDetails,
    MandateDetails,
    PayoutDetails,
    SubscriptionDetails,
    RecoveryRetryDetails,
};

enum class PayoutIdType
{
    PayoutAttemptId,
    ConnectorPayoutId,
};

class PayoutWebhookUpdate
{
public:
    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    std::optional<std::string> error_message{};
    std::optional<std::string> error_code{};
};

class PrimitiveDateTimeValue
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

enum class RefundIdType
{
    RefundId,
    ConnectorRefundId,
};

enum class WebhookFlow
{
    Payment,
    Payout,
    Refund,
    Dispute,
    Subscription,
    ReturnResponse,
    BankTransfer,
    Mandate,
    AssociatedDataUpdate,
    ExternalAuthentication,
    FraudCheck,
    Recovery,
    Setup,
};

enum class WebhookResponseTracker
{
    Payment,
    Payout,
    Refund,
    Dispute,
    Mandate,
    PaymentMethod,
    NoEffect,
    Relay,
};

/**
 * @brief Typed mirror of upstream `WebhookResponse<R>`
 * (hyperswitch_domain_models/src/api.rs:30-46). A connector's webhook acknowledgement
 * is one of these typed variants; the trait default is `StatusOk`
 * (hyperswitch_interfaces/src/webhooks.rs:346-354), which the router sends as HTTP 200
 * with an empty body (router/src/services/api.rs:730-732). Only `response` is on the
 * wire — `ApplicationResponse::IncomingWebhookEvent` metadata is consumed by
 * server_wrap_util for the API event log (router/src/services/api.rs:323-341, :477-485).
 *
 * This is the real typed return contract used by the webhook handler's local
 * `get_webhook_api_response` substitute; it replaces the former private, partial
 * acknowledgement type that carried only three of the five upstream variants.
 */
enum class WebhookResponseKind : std::uint8_t
{
    STATUS_OK,         // WebhookResponse::StatusOk
    JSON,              // WebhookResponse::Json(R)
    TEXT_PLAIN,        // WebhookResponse::TextPlain(String)
    FILE_DATA,         // WebhookResponse::FileData((Vec<u8>, Mime))
    JSON_WITH_HEADERS, // WebhookResponse::JsonWithHeaders((R, Vec<(String, Maskable<String>)>))
};

class WebhookResponse
{
public:
    using Header = std::pair<std::string, std::string>;

    WebhookResponse() = default; // upstream trait default: StatusOk

    [[nodiscard]] static WebhookResponse status_ok() noexcept
    {
        return WebhookResponse{};
    }

    [[nodiscard]] static WebhookResponse json(std::string body)
    {
        WebhookResponse response;
        response.m_kind = WebhookResponseKind::JSON;
        response.m_body = std::move(body);
        return response;
    }

    [[nodiscard]] static WebhookResponse text_plain(std::string body)
    {
        WebhookResponse response;
        response.m_kind = WebhookResponseKind::TEXT_PLAIN;
        response.m_body = std::move(body);
        return response;
    }

    [[nodiscard]] static WebhookResponse file_data(std::vector<std::uint8_t> data, std::string mime)
    {
        WebhookResponse response;
        response.m_kind = WebhookResponseKind::FILE_DATA;
        response.m_file_data = std::move(data);
        response.m_mime = std::move(mime);
        return response;
    }

    [[nodiscard]] static WebhookResponse
    json_with_headers(std::string body, std::vector<Header> headers)
    {
        WebhookResponse response;
        response.m_kind = WebhookResponseKind::JSON_WITH_HEADERS;
        response.m_body = std::move(body);
        response.m_headers = std::move(headers);
        return response;
    }

    void set_kind(WebhookResponseKind value) noexcept
    {
        m_kind = value;
    }

    [[nodiscard]] WebhookResponseKind kind() const noexcept
    {
        return m_kind;
    }

    void set_body(std::string value)
    {
        m_body = std::move(value);
    }

    [[nodiscard]] const std::string& body() const noexcept
    {
        return m_body;
    }

    void set_file_data(std::vector<std::uint8_t> value)
    {
        m_file_data = std::move(value);
    }

    [[nodiscard]] const std::vector<std::uint8_t>& file_data() const noexcept
    {
        return m_file_data;
    }

    void set_mime(std::string value)
    {
        m_mime = std::move(value);
    }

    [[nodiscard]] const std::string& mime() const noexcept
    {
        return m_mime;
    }

    void set_headers(std::vector<Header> value)
    {
        m_headers = std::move(value);
    }

    [[nodiscard]] const std::vector<Header>& headers() const noexcept
    {
        return m_headers;
    }

private:
    WebhookResponseKind m_kind{WebhookResponseKind::STATUS_OK};
    std::string m_body{};
    std::vector<std::uint8_t> m_file_data{};
    std::string m_mime{};
    std::vector<Header> m_headers{};
};

class IncomingWebhookDetails
{
public:
    void set_object_reference_id(ObjectReferenceId value)
    {
        object_reference_id = std::move(value);
    }

    [[nodiscard]] const ObjectReferenceId& get_object_reference_id() const noexcept
    {
        return object_reference_id;
    }

    void set_resource_object(std::vector<std::uint8_t> value)
    {
        resource_object = std::move(value);
    }

    [[nodiscard]] const std::vector<std::uint8_t>& get_resource_object() const noexcept
    {
        return resource_object;
    }

    ObjectReferenceId object_reference_id{};
    std::vector<std::uint8_t> resource_object{};
};

class OutgoingWebhook
{
public:
    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_event_id(std::string value)
    {
        event_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_event_id() const noexcept
    {
        return event_id;
    }

    void set_event_type(EventTypeValue value)
    {
        event_type = std::move(value);
    }

    [[nodiscard]] const EventTypeValue& get_event_type() const noexcept
    {
        return event_type;
    }

    void set_content(OutgoingWebhookContent value)
    {
        content = std::move(value);
    }

    [[nodiscard]] const OutgoingWebhookContent& get_content() const noexcept
    {
        return content;
    }

    void set_timestamp(PrimitiveDateTimeValue value)
    {
        timestamp = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_timestamp() const noexcept
    {
        return timestamp;
    }

    void set_processor_merchant_id(std::optional<MerchantIdValue> value)
    {
        processor_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_processor_merchant_id() const noexcept
    {
        return processor_merchant_id;
    }

    MerchantIdValue merchant_id{};
    std::string event_id{};
    EventTypeValue event_type{};
    OutgoingWebhookContent content{};
    PrimitiveDateTimeValue timestamp{};
    std::optional<MerchantIdValue> processor_merchant_id{};
};
} // namespace app::dto::webhooks

template<>
struct serde::Serializable<app::dto::webhooks::IncomingWebhookDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "object_reference_id",
                &app::dto::webhooks::IncomingWebhookDetails::get_object_reference_id,
                &app::dto::webhooks::IncomingWebhookDetails::set_object_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "resource_object",
                &app::dto::webhooks::IncomingWebhookDetails::get_resource_object,
                &app::dto::webhooks::IncomingWebhookDetails::set_resource_object,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "incoming_webhook_details";
    }
};

template<>
struct serde::Serializable<app::dto::webhooks::PayoutWebhookUpdate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "error_message",
                &app::dto::webhooks::PayoutWebhookUpdate::get_error_message,
                &app::dto::webhooks::PayoutWebhookUpdate::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::dto::webhooks::PayoutWebhookUpdate::get_error_code,
                &app::dto::webhooks::PayoutWebhookUpdate::set_error_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payout_webhook_update";
    }
};

template<>
struct serde::Serializable<app::dto::webhooks::OutgoingWebhook>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::dto::webhooks::OutgoingWebhook::get_merchant_id,
                &app::dto::webhooks::OutgoingWebhook::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_id",
                &app::dto::webhooks::OutgoingWebhook::get_event_id,
                &app::dto::webhooks::OutgoingWebhook::set_event_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_type",
                &app::dto::webhooks::OutgoingWebhook::get_event_type,
                &app::dto::webhooks::OutgoingWebhook::set_event_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "content",
                &app::dto::webhooks::OutgoingWebhook::get_content,
                &app::dto::webhooks::OutgoingWebhook::set_content,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timestamp",
                &app::dto::webhooks::OutgoingWebhook::get_timestamp,
                &app::dto::webhooks::OutgoingWebhook::set_timestamp,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "processor_merchant_id",
                &app::dto::webhooks::OutgoingWebhook::get_processor_merchant_id,
                &app::dto::webhooks::OutgoingWebhook::set_processor_merchant_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "outgoing_webhook";
    }
};

template<>
struct serde::Serializable<app::dto::webhooks::ConnectorWebhookSecrets>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "secret",
                &app::dto::webhooks::ConnectorWebhookSecrets::get_secret,
                &app::dto::webhooks::ConnectorWebhookSecrets::set_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "additional_secret",
                &app::dto::webhooks::ConnectorWebhookSecrets::get_additional_secret,
                &app::dto::webhooks::ConnectorWebhookSecrets::set_additional_secret,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "connector_webhook_secrets";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::webhooks::tests {
using namespace boost::ut;
suite<"app_dto_webhooks"> app_dto_webhooks_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::dto::webhooks::IncomingWebhookDetails>::fields())> > 0
        );
    };
};
} // namespace app::dto::webhooks::tests
#endif
