module;
export module app_dto:webhook_events;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::webhook_events {

class EventClassValue : public core::behavior::Serializer<EventClassValue>
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

enum class EventListConstraintsInternal
{
    GenericFilter,
    ObjectIdFilter,
    EventIdFilter,
};

class EventRecipientValue : public core::behavior::Serializer<EventRecipientValue>
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

class EventTypeValue : public core::behavior::Serializer<EventTypeValue>
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

class MerchantIdValue : public core::behavior::Serializer<MerchantIdValue>
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

class PrimitiveDateTimeValue : public core::behavior::Serializer<PrimitiveDateTimeValue>
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

class ProfileIdValue : public core::behavior::Serializer<ProfileIdValue>
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

class SecretStringValue : public core::behavior::Serializer<SecretStringValue>
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

class WebhookDeliveryAttemptValue : public core::behavior::Serializer<WebhookDeliveryAttemptValue>
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

class EventListConstraints : public core::behavior::Serializer<EventListConstraints>
{
public:
    void set_created_after(std::optional<PrimitiveDateTimeValue> value)
    {
        created_after = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created_after() const noexcept
    {
        return created_after;
    }

    void set_created_before(std::optional<PrimitiveDateTimeValue> value)
    {
        created_before = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created_before() const noexcept
    {
        return created_before;
    }

    void set_limit(std::optional<std::uint16_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint16_t>& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(std::optional<std::uint16_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint16_t>& get_offset() const noexcept
    {
        return offset;
    }

    void set_object_id(std::optional<std::string> value)
    {
        object_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_object_id() const noexcept
    {
        return object_id;
    }

    void set_event_id(std::optional<std::string> value)
    {
        event_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_event_id() const noexcept
    {
        return event_id;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_event_classes(std::optional<std::vector<EventClassValue>> value)
    {
        event_classes = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<EventClassValue>>&
    get_event_classes() const noexcept
    {
        return event_classes;
    }

    void set_event_types(std::optional<std::vector<EventTypeValue>> value)
    {
        event_types = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<EventTypeValue>>& get_event_types() const noexcept
    {
        return event_types;
    }

    void set_is_delivered(std::optional<bool> value)
    {
        is_delivered = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_delivered() const noexcept
    {
        return is_delivered;
    }

    void set_recipient(std::optional<EventRecipientValue> value)
    {
        recipient = std::move(value);
    }

    [[nodiscard]] const std::optional<EventRecipientValue>& get_recipient() const noexcept
    {
        return recipient;
    }

    std::optional<PrimitiveDateTimeValue> created_after{};
    std::optional<PrimitiveDateTimeValue> created_before{};
    std::optional<std::uint16_t> limit{};
    std::optional<std::uint16_t> offset{};
    std::optional<std::string> object_id{};
    std::optional<std::string> event_id{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<std::vector<EventClassValue>> event_classes{};
    std::optional<std::vector<EventTypeValue>> event_types{};
    std::optional<bool> is_delivered{};
    std::optional<EventRecipientValue> recipient{};
};

class EventListItemResponse : public core::behavior::Serializer<EventListItemResponse>
{
public:
    void set_event_id(std::string value)
    {
        event_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_event_id() const noexcept
    {
        return event_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_object_id(std::string value)
    {
        object_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_object_id() const noexcept
    {
        return object_id;
    }

    void set_event_type(EventTypeValue value)
    {
        event_type = std::move(value);
    }

    [[nodiscard]] const EventTypeValue& get_event_type() const noexcept
    {
        return event_type;
    }

    void set_event_class(EventClassValue value)
    {
        event_class = std::move(value);
    }

    [[nodiscard]] const EventClassValue& get_event_class() const noexcept
    {
        return event_class;
    }

    void set_is_delivery_successful(std::optional<bool> value)
    {
        is_delivery_successful = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_delivery_successful() const noexcept
    {
        return is_delivery_successful;
    }

    void set_initial_attempt_id(std::string value)
    {
        initial_attempt_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_initial_attempt_id() const noexcept
    {
        return initial_attempt_id;
    }

    void set_processor_merchant_id(std::optional<MerchantIdValue> value)
    {
        processor_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_processor_merchant_id() const noexcept
    {
        return processor_merchant_id;
    }

    void set_created(PrimitiveDateTimeValue value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created() const noexcept
    {
        return created;
    }

    std::string event_id{};
    MerchantIdValue merchant_id{};
    ProfileIdValue profile_id{};
    std::string object_id{};
    EventTypeValue event_type{};
    EventClassValue event_class{};
    std::optional<bool> is_delivery_successful{};
    std::string initial_attempt_id{};
    std::optional<MerchantIdValue> processor_merchant_id{};
    PrimitiveDateTimeValue created{};
};

class OutgoingWebhookRequestContent :
    public core::behavior::Serializer<OutgoingWebhookRequestContent>
{
public:
    void set_body(SecretStringValue value)
    {
        body = std::move(value);
    }

    [[nodiscard]] const SecretStringValue& get_body() const noexcept
    {
        return body;
    }

    void set_headers(std::vector<std::pair<std::string, SecretStringValue>> value)
    {
        headers = std::move(value);
    }

    [[nodiscard]] const std::vector<std::pair<std::string, SecretStringValue>>&
    get_headers() const noexcept
    {
        return headers;
    }

    SecretStringValue body{};
    std::vector<std::pair<std::string, SecretStringValue>> headers{};
};

class OutgoingWebhookResponseContent :
    public core::behavior::Serializer<OutgoingWebhookResponseContent>
{
public:
    void set_body(std::optional<SecretStringValue> value)
    {
        body = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretStringValue>& get_body() const noexcept
    {
        return body;
    }

    void set_headers(std::optional<std::vector<std::pair<std::string, SecretStringValue>>> value)
    {
        headers = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::pair<std::string, SecretStringValue>>>&
    get_headers() const noexcept
    {
        return headers;
    }

    void set_status_code(std::optional<std::uint16_t> value)
    {
        status_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint16_t>& get_status_code() const noexcept
    {
        return status_code;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    std::optional<SecretStringValue> body{};
    std::optional<std::vector<std::pair<std::string, SecretStringValue>>> headers{};
    std::optional<std::uint16_t> status_code{};
    std::optional<std::string> error_message{};
};

class WebhookDeliveryAttemptListRequestInternal :
    public core::behavior::Serializer<WebhookDeliveryAttemptListRequestInternal>
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

    void set_initial_attempt_id(std::string value)
    {
        initial_attempt_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_initial_attempt_id() const noexcept
    {
        return initial_attempt_id;
    }

    MerchantIdValue merchant_id{};
    std::string initial_attempt_id{};
};

class WebhookDeliveryRetryRequestInternal :
    public core::behavior::Serializer<WebhookDeliveryRetryRequestInternal>
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

    MerchantIdValue merchant_id{};
    std::string event_id{};
};

class EventListRequestInternal : public core::behavior::Serializer<EventListRequestInternal>
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

    void set_constraints(EventListConstraints value)
    {
        constraints = std::move(value);
    }

    [[nodiscard]] const EventListConstraints& get_constraints() const noexcept
    {
        return constraints;
    }

    MerchantIdValue merchant_id{};
    EventListConstraints constraints{};
};

class EventRetrieveResponse : public core::behavior::Serializer<EventRetrieveResponse>
{
public:
    void set_event_information(EventListItemResponse value)
    {
        event_information = std::move(value);
    }

    [[nodiscard]] const EventListItemResponse& get_event_information() const noexcept
    {
        return event_information;
    }

    void set_request(OutgoingWebhookRequestContent value)
    {
        request = std::move(value);
    }

    [[nodiscard]] const OutgoingWebhookRequestContent& get_request() const noexcept
    {
        return request;
    }

    void set_response(OutgoingWebhookResponseContent value)
    {
        response = std::move(value);
    }

    [[nodiscard]] const OutgoingWebhookResponseContent& get_response() const noexcept
    {
        return response;
    }

    void set_delivery_attempt(std::optional<WebhookDeliveryAttemptValue> value)
    {
        delivery_attempt = std::move(value);
    }

    [[nodiscard]] const std::optional<WebhookDeliveryAttemptValue>&
    get_delivery_attempt() const noexcept
    {
        return delivery_attempt;
    }

    EventListItemResponse event_information{};
    OutgoingWebhookRequestContent request{};
    OutgoingWebhookResponseContent response{};
    std::optional<WebhookDeliveryAttemptValue> delivery_attempt{};
};

class TotalEventsResponse : public core::behavior::Serializer<TotalEventsResponse>
{
public:
    void set_events(std::vector<EventListItemResponse> value)
    {
        events = std::move(value);
    }

    [[nodiscard]] const std::vector<EventListItemResponse>& get_events() const noexcept
    {
        return events;
    }

    void set_total_count(std::int64_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_total_count() const noexcept
    {
        return total_count;
    }

    std::vector<EventListItemResponse> events{};
    std::int64_t total_count{};
};

// Typed result of the upstream list_initial_delivery_attempts core function:
// the page of typed EventListItemResponse rows and the separate COUNT. This is
// the real return type used by the handler (no local ad-hoc placeholder).
class EventListResult : public core::behavior::Serializer<EventListResult>
{
public:
    void set_items(std::vector<EventListItemResponse> value)
    {
        items = std::move(value);
    }

    [[nodiscard]] const std::vector<EventListItemResponse>& get_items() const noexcept
    {
        return items;
    }

    void set_total_count(std::int64_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_total_count() const noexcept
    {
        return total_count;
    }

    std::vector<EventListItemResponse> items{};
    std::int64_t total_count{0};
};
} // namespace app::dto::webhook_events

template<>
struct serde::Serializable<app::dto::webhook_events::EventListConstraints>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "created_after",
                &app::dto::webhook_events::EventListConstraints::get_created_after,
                &app::dto::webhook_events::EventListConstraints::set_created_after,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_before",
                &app::dto::webhook_events::EventListConstraints::get_created_before,
                &app::dto::webhook_events::EventListConstraints::set_created_before,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::dto::webhook_events::EventListConstraints::get_limit,
                &app::dto::webhook_events::EventListConstraints::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::dto::webhook_events::EventListConstraints::get_offset,
                &app::dto::webhook_events::EventListConstraints::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "object_id",
                &app::dto::webhook_events::EventListConstraints::get_object_id,
                &app::dto::webhook_events::EventListConstraints::set_object_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_id",
                &app::dto::webhook_events::EventListConstraints::get_event_id,
                &app::dto::webhook_events::EventListConstraints::set_event_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::dto::webhook_events::EventListConstraints::get_profile_id,
                &app::dto::webhook_events::EventListConstraints::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_classes",
                &app::dto::webhook_events::EventListConstraints::get_event_classes,
                &app::dto::webhook_events::EventListConstraints::set_event_classes,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_types",
                &app::dto::webhook_events::EventListConstraints::get_event_types,
                &app::dto::webhook_events::EventListConstraints::set_event_types,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_delivered",
                &app::dto::webhook_events::EventListConstraints::get_is_delivered,
                &app::dto::webhook_events::EventListConstraints::set_is_delivered,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recipient",
                &app::dto::webhook_events::EventListConstraints::get_recipient,
                &app::dto::webhook_events::EventListConstraints::set_recipient,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "event_list_constraints";
    }
};

template<>
struct serde::Serializable<app::dto::webhook_events::EventListItemResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "event_id",
                &app::dto::webhook_events::EventListItemResponse::get_event_id,
                &app::dto::webhook_events::EventListItemResponse::set_event_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::dto::webhook_events::EventListItemResponse::get_merchant_id,
                &app::dto::webhook_events::EventListItemResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::dto::webhook_events::EventListItemResponse::get_profile_id,
                &app::dto::webhook_events::EventListItemResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "object_id",
                &app::dto::webhook_events::EventListItemResponse::get_object_id,
                &app::dto::webhook_events::EventListItemResponse::set_object_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_type",
                &app::dto::webhook_events::EventListItemResponse::get_event_type,
                &app::dto::webhook_events::EventListItemResponse::set_event_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_class",
                &app::dto::webhook_events::EventListItemResponse::get_event_class,
                &app::dto::webhook_events::EventListItemResponse::set_event_class,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_delivery_successful",
                &app::dto::webhook_events::EventListItemResponse::get_is_delivery_successful,
                &app::dto::webhook_events::EventListItemResponse::set_is_delivery_successful,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "initial_attempt_id",
                &app::dto::webhook_events::EventListItemResponse::get_initial_attempt_id,
                &app::dto::webhook_events::EventListItemResponse::set_initial_attempt_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "processor_merchant_id",
                &app::dto::webhook_events::EventListItemResponse::get_processor_merchant_id,
                &app::dto::webhook_events::EventListItemResponse::set_processor_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::dto::webhook_events::EventListItemResponse::get_created,
                &app::dto::webhook_events::EventListItemResponse::set_created,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "event_list_item_response";
    }
};

template<>
struct serde::Serializable<app::dto::webhook_events::TotalEventsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "events",
                &app::dto::webhook_events::TotalEventsResponse::get_events,
                &app::dto::webhook_events::TotalEventsResponse::set_events,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::dto::webhook_events::TotalEventsResponse::get_total_count,
                &app::dto::webhook_events::TotalEventsResponse::set_total_count,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "total_events_response";
    }
};

template<>
struct serde::Serializable<app::dto::webhook_events::EventListResult>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "items",
                &app::dto::webhook_events::EventListResult::get_items,
                &app::dto::webhook_events::EventListResult::set_items,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::dto::webhook_events::EventListResult::get_total_count,
                &app::dto::webhook_events::EventListResult::set_total_count,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "event_list_result";
    }
};

template<>
struct serde::Serializable<app::dto::webhook_events::EventRetrieveResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "event_information",
                &app::dto::webhook_events::EventRetrieveResponse::get_event_information,
                &app::dto::webhook_events::EventRetrieveResponse::set_event_information,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "request",
                &app::dto::webhook_events::EventRetrieveResponse::get_request,
                &app::dto::webhook_events::EventRetrieveResponse::set_request,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "response",
                &app::dto::webhook_events::EventRetrieveResponse::get_response,
                &app::dto::webhook_events::EventRetrieveResponse::set_response,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delivery_attempt",
                &app::dto::webhook_events::EventRetrieveResponse::get_delivery_attempt,
                &app::dto::webhook_events::EventRetrieveResponse::set_delivery_attempt,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "event_retrieve_response";
    }
};

template<>
struct serde::Serializable<app::dto::webhook_events::OutgoingWebhookRequestContent>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "body",
                &app::dto::webhook_events::OutgoingWebhookRequestContent::get_body,
                &app::dto::webhook_events::OutgoingWebhookRequestContent::set_body,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "headers",
                &app::dto::webhook_events::OutgoingWebhookRequestContent::get_headers,
                &app::dto::webhook_events::OutgoingWebhookRequestContent::set_headers,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "outgoing_webhook_request_content";
    }
};

template<>
struct serde::Serializable<app::dto::webhook_events::OutgoingWebhookResponseContent>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "body",
                &app::dto::webhook_events::OutgoingWebhookResponseContent::get_body,
                &app::dto::webhook_events::OutgoingWebhookResponseContent::set_body,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "headers",
                &app::dto::webhook_events::OutgoingWebhookResponseContent::get_headers,
                &app::dto::webhook_events::OutgoingWebhookResponseContent::set_headers,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status_code",
                &app::dto::webhook_events::OutgoingWebhookResponseContent::get_status_code,
                &app::dto::webhook_events::OutgoingWebhookResponseContent::set_status_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::dto::webhook_events::OutgoingWebhookResponseContent::get_error_message,
                &app::dto::webhook_events::OutgoingWebhookResponseContent::set_error_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "outgoing_webhook_response_content";
    }
};

template<>
struct serde::Serializable<app::dto::webhook_events::EventListRequestInternal>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::dto::webhook_events::EventListRequestInternal::get_merchant_id,
                &app::dto::webhook_events::EventListRequestInternal::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "constraints",
                &app::dto::webhook_events::EventListRequestInternal::get_constraints,
                &app::dto::webhook_events::EventListRequestInternal::set_constraints,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "event_list_request_internal";
    }
};

template<>
struct serde::Serializable<app::dto::webhook_events::WebhookDeliveryAttemptListRequestInternal>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::dto::webhook_events::WebhookDeliveryAttemptListRequestInternal::
                    get_merchant_id,
                &app::dto::webhook_events::WebhookDeliveryAttemptListRequestInternal::
                    set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "initial_attempt_id",
                &app::dto::webhook_events::WebhookDeliveryAttemptListRequestInternal::
                    get_initial_attempt_id,
                &app::dto::webhook_events::WebhookDeliveryAttemptListRequestInternal::
                    set_initial_attempt_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "webhook_delivery_attempt_list_request_internal";
    }
};

template<>
struct serde::Serializable<app::dto::webhook_events::WebhookDeliveryRetryRequestInternal>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::dto::webhook_events::WebhookDeliveryRetryRequestInternal::get_merchant_id,
                &app::dto::webhook_events::WebhookDeliveryRetryRequestInternal::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_id",
                &app::dto::webhook_events::WebhookDeliveryRetryRequestInternal::get_event_id,
                &app::dto::webhook_events::WebhookDeliveryRetryRequestInternal::set_event_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "webhook_delivery_retry_request_internal";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::webhook_events::tests {
using namespace boost::ut;
suite<"app_dto_webhook_events"> app_dto_webhook_events_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::dto::webhook_events::EventListConstraints>::fields())> >
            0
        );
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::dto::webhook_events::EventListResult>::fields())> == 2
        );
    };
};
} // namespace app::dto::webhook_events::tests
#endif
