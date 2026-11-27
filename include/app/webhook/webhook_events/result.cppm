export module app_webhook:webhook_events_result;

import std;
import interfaces;
import core_behavior;
import core_serde;
import core_error;
import app_dto;

export namespace app::handlers::webhook_events {

// Wire field lists captured from the retired Result serializers, in declaration
// order. Serializer<T>::serialize(fields) emits the same upstream bytes.
[[nodiscard]] inline constexpr auto total_events_wire_fields()
{
    return std::array<std::string_view, 2>{"events", "total_count"};
}

[[nodiscard]] inline constexpr auto event_retrieve_wire_fields()
{
    return std::array<std::string_view, 4>{
        "event_information",
        "request",
        "response",
        "delivery_attempt"
    };
}

} // namespace app::handlers::webhook_events

namespace {

void publish_event_value(
    interfaces::io::ResponseSink& sink,
    const std::expected<std::string, core::error::AppError>& encoded
)
{
    if (!encoded) {
        encoded.error().send(sink);
        return;
    }
    sink.set_status(interfaces::io::types::Status::OK);
    std::vector<std::byte> body_bytes{};
    body_bytes.reserve(encoded->size());
    for (const char ch: *encoded) {
        body_bytes.push_back(static_cast<std::byte>(ch));
    }
    sink.set_body(std::move(body_bytes));
    sink.send();
}

} // namespace

export template<>
class core::behavior::Outcome<::app::dto::webhook_events::TotalEventsResponse> :
    public std::expected<::app::dto::webhook_events::TotalEventsResponse, core::error::AppError>
{
public:
    using std::expected<::app::dto::webhook_events::TotalEventsResponse, core::error::AppError>::
        expected;

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!has_value()) {
            error().send(sink);
            return;
        }
        publish_event_value(
            sink,
            value().serialize(app::handlers::webhook_events::total_events_wire_fields())
        );
    }
};

export template<>
class core::behavior::Outcome<::app::dto::webhook_events::EventRetrieveResponse> :
    public std::expected<::app::dto::webhook_events::EventRetrieveResponse, core::error::AppError>
{
public:
    using std::expected<::app::dto::webhook_events::EventRetrieveResponse, core::error::AppError>::
        expected;

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!has_value()) {
            error().send(sink);
            return;
        }
        publish_event_value(
            sink,
            value().serialize(app::handlers::webhook_events::event_retrieve_wire_fields())
        );
    }
};

export template<>
class core::behavior::Outcome<std::vector<::app::dto::webhook_events::EventRetrieveResponse>> :
    public std::expected<
        std::vector<::app::dto::webhook_events::EventRetrieveResponse>,
        core::error::AppError>
{
public:
    using std::expected<
        std::vector<::app::dto::webhook_events::EventRetrieveResponse>,
        core::error::AppError>::expected;

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!has_value()) {
            error().send(sink);
            return;
        }
        using Dto = ::app::dto::webhook_events::EventRetrieveResponse;
        publish_event_value(
            sink,
            serde::Serializer<std::vector<Dto>>::serialize_values(
                value(),
                app::handlers::webhook_events::event_retrieve_wire_fields()
            )
        );
    }
};
