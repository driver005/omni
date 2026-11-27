export module app_webhook:webhook_events_result;

import std;

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
