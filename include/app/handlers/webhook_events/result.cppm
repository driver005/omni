export module app_handlers_webhook_events:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::webhook_events {

// Routes 1 and 2 keep the exact TotalEventsResponse wire shape (events,
// total_count). The Serializable specialization reuses the DTO's FieldDesc
// tuple, so field names and order stay byte-identical.
class ListInitialWithJwtAuthResult : public app::dto::webhook_events::TotalEventsResponse
{
public:
    ListInitialWithJwtAuthResult() = default;

    explicit ListInitialWithJwtAuthResult(app::dto::webhook_events::EventListResult value)
    {
        set_total_count(value.total_count);
        set_events(std::move(value.items));
    }
};

class ListInitialResult : public app::dto::webhook_events::TotalEventsResponse
{
public:
    ListInitialResult() = default;

    explicit ListInitialResult(app::dto::webhook_events::EventListResult value)
    {
        set_total_count(value.total_count);
        set_events(std::move(value.items));
    }
};

// The attempts route returns a top-level JSON array of EventRetrieveResponse.
// The result carries the typed vector; the handler serializes get_items() so
// the wire stays a bare array without any envelope.
class ListAttemptsResult
{
public:
    ListAttemptsResult() = default;

    explicit ListAttemptsResult(
        std::vector<app::dto::webhook_events::EventRetrieveResponse> items
    ) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<app::dto::webhook_events::EventRetrieveResponse>&
    get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<app::dto::webhook_events::EventRetrieveResponse> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<app::dto::webhook_events::EventRetrieveResponse> m_items{};
};

// Retry returns a single EventRetrieveResponse document. Keep it typed so the
// handler can pass the DTO straight to Ser::serialize.
class RetryResult
{
public:
    RetryResult() = default;

    explicit RetryResult(app::dto::webhook_events::EventRetrieveResponse event) noexcept :
        m_event{std::move(event)}
    {
    }

    [[nodiscard]] const app::dto::webhook_events::EventRetrieveResponse& get_event() const noexcept
    {
        return m_event;
    }

    void set_event(app::dto::webhook_events::EventRetrieveResponse event)
    {
        m_event = std::move(event);
    }

private:
    app::dto::webhook_events::EventRetrieveResponse m_event{};
};

} // namespace app::handlers::webhook_events

template<>
struct serde::Serializable<app::handlers::webhook_events::ListInitialWithJwtAuthResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::webhook_events::TotalEventsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "list_initial_with_jwt_auth_result";
    }
};

template<>
struct serde::Serializable<app::handlers::webhook_events::ListInitialResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::webhook_events::TotalEventsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "list_initial_result";
    }
};
