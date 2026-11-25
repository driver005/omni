module;
export module app_workflow:process_tracker_service;

import std;
import core_database;
import core_error;
import app_metrics;
import :process_tracker_status;
import :process_tracker_record;
import :retry_policy;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::workflow {

/** @brief Schedules, looks up and advances process-tracker records. */
class ProcessTrackerService
{
public:
    ProcessTrackerService(core::db::Database& db, app::metrics::MetricsRegistry& metrics) :
        m_db{db},
        m_metrics{metrics}
    {
    }

    /** @brief Persists a new tracker and returns its generated record id. */
    std::int64_t schedule(std::string name, std::string payload, std::string run_at)
    {
        ProcessTrackerRecord record{};
        record.set_name(std::move(name));
        record.set_payload(std::move(payload));
        record.set_run_at(std::move(run_at));
        record.set_status(std::string{ProcessTrackerStatus{}.pending()});
        record.set_attempts(0);
        record.set_max_attempts(3);
        record.set_created_at(now_string(0));
        record.set_updated_at(record.get_created_at());

        const ProcessTrackerRecord stored = m_db.insert<ProcessTrackerRecord>(std::move(record));
        m_metrics.counter("process_tracker_scheduled").increment();
        m_metrics.record_flow("process_tracker.schedule");
        m_last_error.reset();
        return stored.get_id();
    }

    [[nodiscard]] std::optional<ProcessTrackerRecord> find_by_id(std::int64_t id)
    {
        std::optional<ProcessTrackerRecord> found =
            m_db.find<ProcessTrackerRecord>(core::db::DbValue{id});
        if (!found) {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "process tracker not found"
            );
        } else {
            m_last_error.reset();
        }
        return found;
    }

    bool update_status(std::int64_t id, std::string status)
    {
        std::optional<ProcessTrackerRecord> record =
            m_db.find<ProcessTrackerRecord>(core::db::DbValue{id});
        if (!record) {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "process tracker not found"
            );
            return false;
        }
        record->set_status(std::move(status));
        record->set_updated_at(now_string(0));
        const bool updated = m_db.update_where_column<ProcessTrackerRecord>(
                                 *record,
                                 "id",
                                 core::db::Compare::EQ,
                                 core::db::DbValue{id}
                             ) > 0;
        if (updated) {
            m_metrics.counter("process_tracker_status_updated").increment();
            m_last_error.reset();
        } else {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "process tracker update failed"
            );
        }
        return updated;
    }

    /** @brief Re-arms a tracker with the next attempt and backoff from the policy. */
    std::optional<ProcessTrackerRecord> retry(std::int64_t id, const RetryPolicy& policy)
    {
        std::optional<ProcessTrackerRecord> record =
            m_db.find<ProcessTrackerRecord>(core::db::DbValue{id});
        if (!record) {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "process tracker not found"
            );
            return std::nullopt;
        }
        if (!policy.should_retry(record->get_attempts())) {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "process tracker retry limit reached"
            );
            return std::nullopt;
        }

        const std::int64_t attempts = record->get_attempts() + 1;
        record->set_attempts(attempts);
        record->set_status(std::string{ProcessTrackerStatus{}.retrying()});
        record->set_run_at(now_string(policy.delay_seconds(attempts)));
        record->set_updated_at(now_string(0));
        m_db.update_where_column<ProcessTrackerRecord>(
            *record,
            "id",
            core::db::Compare::EQ,
            core::db::DbValue{id}
        );
        m_metrics.counter("process_tracker_retried").increment();
        m_last_error.reset();
        return record;
    }

    bool mark_terminal(std::int64_t id, std::string result)
    {
        std::optional<ProcessTrackerRecord> record =
            m_db.find<ProcessTrackerRecord>(core::db::DbValue{id});
        if (!record) {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "process tracker not found"
            );
            return false;
        }
        record->set_status(std::string{ProcessTrackerStatus{}.completed()});
        record->set_result(std::move(result));
        record->set_updated_at(now_string(0));
        const bool updated = m_db.update_where_column<ProcessTrackerRecord>(
                                 *record,
                                 "id",
                                 core::db::Compare::EQ,
                                 core::db::DbValue{id}
                             ) > 0;
        if (updated) {
            m_metrics.counter("process_tracker_terminal").increment();
            m_last_error.reset();
        } else {
            m_last_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "process tracker terminal update failed"
            );
        }
        return updated;
    }

    [[nodiscard]] std::optional<core::error::AppError> last_error() const noexcept
    {
        return m_last_error;
    }

private:
    [[nodiscard]] std::string now_string(std::int64_t offset_seconds) const
    {
        const std::int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
                                     std::chrono::system_clock::now().time_since_epoch()
        )
                                     .count();
        return std::to_string(now + offset_seconds);
    }

    core::db::Database& m_db;
    app::metrics::MetricsRegistry& m_metrics;
    std::optional<core::error::AppError> m_last_error{};
};

} // namespace app::workflow

#ifdef CONGELADO_TEST
namespace app::workflow::tests {

using namespace boost::ut;

suite<"app_workflow_process_tracker_service"> process_tracker_service_suite = []
{
    "schedules, reads and advances a tracker through its lifecycle"_test = []
    {
        core::db::Database db = core::db::DatabaseBuilder{}
                                    .engine(core::db::StorageEngine::IN_MEMORY)
                                    .register_entity<ProcessTrackerRecord>()
                                    .build();
        app::metrics::MetricsRegistry metrics{};
        ProcessTrackerService service{db, metrics};

        const std::int64_t id = service.schedule("api_key_expiry", "key_1", "0");
        expect(id > 0);

        const auto scheduled = service.find_by_id(id);
        expect(scheduled.has_value());
        expect(scheduled->get_status() == "PENDING");

        expect(service.update_status(id, "RUNNING"));
        expect(service.find_by_id(id)->get_status() == "RUNNING");

        RetryPolicy policy{};
        const auto retried = service.retry(id, policy);
        expect(retried.has_value());
        expect(retried->get_attempts() == 1);
        expect(retried->get_status() == "RETRYING");

        expect(service.mark_terminal(id, "done"));
        const auto terminal = service.find_by_id(id);
        expect(terminal.has_value());
        expect(terminal->get_status() == "COMPLETED");
        expect(terminal->get_result() == "done");
        expect(metrics.counter("process_tracker_terminal").value() == 1);
    };

    "reports a missing tracker as NOT_FOUND"_test = []
    {
        core::db::Database db = core::db::DatabaseBuilder{}
                                    .engine(core::db::StorageEngine::IN_MEMORY)
                                    .register_entity<ProcessTrackerRecord>()
                                    .build();
        app::metrics::MetricsRegistry metrics{};
        ProcessTrackerService service{db, metrics};

        expect(!service.find_by_id(999).has_value());
        expect(service.last_error().has_value());
        expect(service.last_error()->get_kind() == core::error::AppErrorKind::NOT_FOUND);
    };
};

} // namespace app::workflow::tests
#endif
