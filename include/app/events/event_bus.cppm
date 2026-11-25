module;
export module app_events:event_bus;

import std;
import core_database;
import core_error;
import app_metrics;
import :event_record;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::events {

/** @brief Records Kafka-shaped events with a deterministic id and a named metric. */
class EventBus
{
public:
    EventBus(core::db::Database& db, app::metrics::MetricsRegistry& metrics) :
        m_db{db},
        m_metrics{metrics}
    {
    }

    /** @brief Persists the envelope and returns its deterministic id (`evt_<row id>`). */
    std::string publish(std::string topic, std::string payload)
    {
        EventRecord record{};
        record.set_topic(std::move(topic));
        record.set_payload(std::move(payload));
        record.set_status("PUBLISHED");
        record.set_attempts(0);
        record.set_created_at(now_string());
        record.set_updated_at(record.get_created_at());

        EventRecord stored = m_db.insert<EventRecord>(std::move(record));
        const std::string event_id = std::format("evt_{}", stored.get_id());
        stored.set_event_id(event_id);
        m_db.update_where_column<EventRecord>(
            stored,
            "id",
            core::db::Compare::EQ,
            core::db::DbValue{stored.get_id()}
        );

        m_metrics.counter("event_published").increment();
        m_metrics.record_flow("event.publish");
        m_last_error.reset();
        return event_id;
    }

    [[nodiscard]] std::optional<EventRecord> find_by_event_id(std::string_view event_id)
    {
        core::db::QuerySpec spec = m_db.query<EventRecord>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "event_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{event_id}},
            }
        );
        spec.limit_count = 1;
        const std::vector<EventRecord> records = m_db.collect<EventRecord>(spec);
        if (records.empty()) {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "event not found"
            );
            return std::nullopt;
        }
        m_last_error.reset();
        return records.front();
    }

    bool save(const EventRecord& record)
    {
        const bool saved = m_db.update_where_column<EventRecord>(
                               record,
                               "id",
                               core::db::Compare::EQ,
                               core::db::DbValue{record.get_id()}
                           ) > 0;
        if (!saved) {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "event update failed"
            );
        }
        return saved;
    }

    [[nodiscard]] std::optional<core::error::AppError> last_error() const noexcept
    {
        return m_last_error;
    }

private:
    [[nodiscard]] std::string now_string() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            )
                .count()
        );
    }

    core::db::Database& m_db;
    app::metrics::MetricsRegistry& m_metrics;
    std::optional<core::error::AppError> m_last_error{};
};

} // namespace app::events

#ifdef CONGELADO_TEST
namespace app::events::tests {

using namespace boost::ut;

suite<"app_events_event_bus"> event_bus_suite = []
{
    "publishes a deterministic event id and persists the envelope"_test = []
    {
        core::db::Database db = core::db::DatabaseBuilder{}
                                    .engine(core::db::StorageEngine::IN_MEMORY)
                                    .register_entity<EventRecord>()
                                    .build();
        app::metrics::MetricsRegistry metrics{};
        EventBus bus{db, metrics};

        const std::string event_id = bus.publish("payment.intent.created", "{\"id\":\"pi_1\"}");
        expect(!event_id.empty());
        expect(event_id.starts_with("evt_"));

        const auto stored = bus.find_by_event_id(event_id);
        expect(stored.has_value());
        expect(stored->get_topic() == "payment.intent.created");
        expect(stored->get_status() == "PUBLISHED");
        expect(stored->get_event_id() == event_id);
        expect(metrics.counter("event_published").value() == 1);
    };

    "reports a missing event as NOT_FOUND"_test = []
    {
        core::db::Database db = core::db::DatabaseBuilder{}
                                    .engine(core::db::StorageEngine::IN_MEMORY)
                                    .register_entity<EventRecord>()
                                    .build();
        app::metrics::MetricsRegistry metrics{};
        EventBus bus{db, metrics};

        expect(!bus.find_by_event_id("evt_missing").has_value());
        expect(bus.last_error().has_value());
        expect(bus.last_error()->get_kind() == core::error::AppErrorKind::NOT_FOUND);
    };
};

} // namespace app::events::tests
#endif
