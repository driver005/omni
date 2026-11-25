module;
export module app_events:event_record;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::events {

/** @brief Persisted Kafka-shaped event envelope with a deterministic event id. */
class EventRecord
{
public:
    EventRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_event_id(std::string value)
    {
        m_event_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_event_id() const noexcept
    {
        return m_event_id;
    }

    void set_topic(std::string value)
    {
        m_topic = std::move(value);
    }

    [[nodiscard]] const std::string& get_topic() const noexcept
    {
        return m_topic;
    }

    void set_payload(std::string value)
    {
        m_payload = std::move(value);
    }

    [[nodiscard]] const std::string& get_payload() const noexcept
    {
        return m_payload;
    }

    void set_endpoint(std::string value)
    {
        m_endpoint = std::move(value);
    }

    [[nodiscard]] const std::string& get_endpoint() const noexcept
    {
        return m_endpoint;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_attempts(std::int64_t value)
    {
        m_attempts = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_attempts() const noexcept
    {
        return m_attempts;
    }

    void set_created_at(std::string value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return m_created_at;
    }

    void set_updated_at(std::string value)
    {
        m_updated_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_updated_at() const noexcept
    {
        return m_updated_at;
    }

    std::int64_t m_id{};
    std::string m_event_id{};
    std::string m_topic{};
    std::string m_payload{};
    std::string m_endpoint{};
    std::string m_status{};
    std::int64_t m_attempts{};
    std::string m_created_at{};
    std::string m_updated_at{};
};

} // namespace app::events

template<>
struct serde::Serializable<app::events::EventRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::events::EventRecord::get_id,
                &app::events::EventRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "event_id",
                &app::events::EventRecord::get_event_id,
                &app::events::EventRecord::set_event_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "topic",
                &app::events::EventRecord::get_topic,
                &app::events::EventRecord::set_topic,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payload",
                &app::events::EventRecord::get_payload,
                &app::events::EventRecord::set_payload,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "endpoint",
                &app::events::EventRecord::get_endpoint,
                &app::events::EventRecord::set_endpoint,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::events::EventRecord::get_status,
                &app::events::EventRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempts",
                &app::events::EventRecord::get_attempts,
                &app::events::EventRecord::set_attempts,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::events::EventRecord::get_created_at,
                &app::events::EventRecord::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_at",
                &app::events::EventRecord::get_updated_at,
                &app::events::EventRecord::set_updated_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "events";
    }
};

#ifdef CONGELADO_TEST
namespace app::events::tests {

using namespace boost::ut;

suite<"app_events_event_record"> event_record_suite = []
{
    "stores envelope fields and reports its table"_test = []
    {
        EventRecord record{};
        record.set_event_id("evt_1");
        record.set_topic("payment.intent.created");
        record.set_payload("{}");
        record.set_status("PUBLISHED");

        expect(record.get_event_id() == "evt_1");
        expect(record.get_topic() == "payment.intent.created");
        expect(record.get_status() == "PUBLISHED");
        expect(serde::Serializable<EventRecord>::table_name() == "events");
    };
};

} // namespace app::events::tests
#endif
