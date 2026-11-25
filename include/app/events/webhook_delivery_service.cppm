module;
export module app_events:webhook_delivery_service;

import std;
import core_database;
import core_error;
import app_metrics;
import app_workflow;
import :event_bus;
import :event_record;
import :webhook_delivery_attempt;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::events {

/** @brief Queues, attempts and retries webhook deliveries against persisted events. */
class WebhookDeliveryService
{
public:
    WebhookDeliveryService(
        core::db::Database& db,
        app::metrics::MetricsRegistry& metrics,
        app::workflow::ProcessTrackerService& trackers,
        EventBus& events
    ) :
        m_db{db},
        m_metrics{metrics},
        m_trackers{trackers},
        m_events{events}
    {
    }

    /** @brief Marks an event queued and schedules a process-tracker delivery task. */
    bool enqueue(std::string event_id, std::string endpoint)
    {
        std::optional<EventRecord> event = m_events.find_by_event_id(event_id);
        if (!event) {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "event not found"
            );
            return false;
        }
        const std::string payload =
            std::format("{{\"event_id\":\"{}\",\"endpoint\":\"{}\"}}", event_id, endpoint);
        const std::int64_t tracker_id = m_trackers.schedule("webhook_delivery", payload, "0");
        m_tracker_ids.insert_or_assign(event_id, tracker_id);

        event->set_endpoint(std::move(endpoint));
        event->set_status("QUEUED");
        event->set_updated_at(now_string());
        m_events.save(*event);

        m_metrics.counter("webhook_delivery_enqueued").increment();
        m_metrics.record_flow("webhook_delivery.enqueue");
        m_last_error.reset();
        return true;
    }

    /** @brief Performs one deterministic local delivery attempt and records it. */
    bool attempt(std::string event_id)
    {
        std::optional<EventRecord> event = m_events.find_by_event_id(event_id);
        if (!event) {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "event not found"
            );
            return false;
        }
        if (event->get_status() == "DELIVERED") {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "event already delivered"
            );
            return false;
        }

        const std::string endpoint = event->get_endpoint();
        const bool success = !endpoint.empty();
        const std::string response = success ? "local delivery accepted" : "missing endpoint";
        record_attempt(event_id, endpoint, success, response);

        const std::int64_t attempts = event->get_attempts() + 1;
        event->set_attempts(attempts);
        event->set_status(success ? "DELIVERED" : "FAILED");
        event->set_updated_at(now_string());
        m_events.save(*event);

        if (success) {
            const auto tracked = m_tracker_ids.find(event_id);
            if (tracked != m_tracker_ids.end()) {
                m_trackers.mark_terminal(tracked->second, "delivered");
            }
        }
        m_metrics.counter("webhook_delivery_attempted").increment();
        m_last_error.reset();
        return success;
    }

    bool record_attempt(
        std::string event_id,
        std::string endpoint,
        bool success,
        std::string response
    )
    {
        WebhookDeliveryAttemptRecord attempt{};
        attempt.set_event_id(std::move(event_id));
        attempt.set_endpoint(std::move(endpoint));
        attempt.set_status(success ? "SUCCESS" : "FAILED");
        attempt.set_response(std::move(response));
        attempt.set_created_at(now_string());
        m_db.insert<WebhookDeliveryAttemptRecord>(std::move(attempt));
        m_metrics.counter("webhook_delivery_attempt_recorded").increment();
        return success;
    }

    /** @brief Re-arms the queued delivery through the process-tracker service. */
    bool retry(std::string event_id, const app::workflow::RetryPolicy& policy)
    {
        std::optional<EventRecord> event = m_events.find_by_event_id(event_id);
        if (!event) {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "event not found"
            );
            return false;
        }
        const auto tracked = m_tracker_ids.find(event_id);
        if (tracked == m_tracker_ids.end()) {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "event has no queued delivery"
            );
            return false;
        }
        const auto retried = m_trackers.retry(tracked->second, policy);
        if (!retried) {
            m_last_error = m_trackers.last_error();
            return false;
        }

        event->set_status("RETRYING");
        event->set_updated_at(now_string());
        m_events.save(*event);
        m_metrics.counter("webhook_delivery_retried").increment();
        m_last_error.reset();
        return true;
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
    app::workflow::ProcessTrackerService& m_trackers;
    EventBus& m_events;
    std::map<std::string, std::int64_t> m_tracker_ids{};
    std::optional<core::error::AppError> m_last_error{};
};

} // namespace app::events

#ifdef CONGELADO_TEST
namespace app::events::tests {

using namespace boost::ut;

suite<"app_events_webhook_delivery_service"> webhook_delivery_service_suite = []
{
    "enqueues and delivers a webhook end to end"_test = []
    {
        core::db::Database db = core::db::DatabaseBuilder{}
                                    .engine(core::db::StorageEngine::IN_MEMORY)
                                    .register_entity<EventRecord>()
                                    .register_entity<WebhookDeliveryAttemptRecord>()
                                    .register_entity<app::workflow::ProcessTrackerRecord>()
                                    .build();
        app::metrics::MetricsRegistry metrics{};
        app::workflow::ProcessTrackerService trackers{db, metrics};
        EventBus bus{db, metrics};
        WebhookDeliveryService delivery{db, metrics, trackers, bus};

        const std::string event_id = bus.publish("payment.intent.created", "{}");
        expect(delivery.enqueue(event_id, "https://merchant.example/hook"));
        expect(bus.find_by_event_id(event_id)->get_status() == "QUEUED");
        expect(delivery.attempt(event_id));
        expect(bus.find_by_event_id(event_id)->get_status() == "DELIVERED");
        expect(metrics.counter("webhook_delivery_attempt_recorded").value() == 1);
    };

    "reports a missing event as NOT_FOUND"_test = []
    {
        core::db::Database db = core::db::DatabaseBuilder{}
                                    .engine(core::db::StorageEngine::IN_MEMORY)
                                    .register_entity<EventRecord>()
                                    .register_entity<WebhookDeliveryAttemptRecord>()
                                    .register_entity<app::workflow::ProcessTrackerRecord>()
                                    .build();
        app::metrics::MetricsRegistry metrics{};
        app::workflow::ProcessTrackerService trackers{db, metrics};
        EventBus bus{db, metrics};
        WebhookDeliveryService delivery{db, metrics, trackers, bus};

        expect(!delivery.enqueue("evt_missing", "https://merchant.example/hook"));
        expect(delivery.last_error().has_value());
        expect(delivery.last_error()->get_kind() == core::error::AppErrorKind::NOT_FOUND);
    };
};

} // namespace app::events::tests
#endif
