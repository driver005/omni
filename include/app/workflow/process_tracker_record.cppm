module;
export module app_workflow:process_tracker_record;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::workflow {

/** @brief Persisted record describing one scheduled unit of background work. */
class ProcessTrackerRecord
{
public:
    ProcessTrackerRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_name(std::string value)
    {
        m_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    void set_payload(std::string value)
    {
        m_payload = std::move(value);
    }

    [[nodiscard]] const std::string& get_payload() const noexcept
    {
        return m_payload;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_run_at(std::string value)
    {
        m_run_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_run_at() const noexcept
    {
        return m_run_at;
    }

    void set_attempts(std::int64_t value)
    {
        m_attempts = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_attempts() const noexcept
    {
        return m_attempts;
    }

    void set_max_attempts(std::int64_t value)
    {
        m_max_attempts = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_max_attempts() const noexcept
    {
        return m_max_attempts;
    }

    void set_result(std::string value)
    {
        m_result = std::move(value);
    }

    [[nodiscard]] const std::string& get_result() const noexcept
    {
        return m_result;
    }

    void set_error(std::string value)
    {
        m_error = std::move(value);
    }

    [[nodiscard]] const std::string& get_error() const noexcept
    {
        return m_error;
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
    std::string m_name{};
    std::string m_payload{};
    std::string m_status{};
    std::string m_run_at{};
    std::int64_t m_attempts{};
    std::int64_t m_max_attempts{};
    std::string m_result{};
    std::string m_error{};
    std::string m_created_at{};
    std::string m_updated_at{};
};

} // namespace app::workflow

template<>
struct serde::Serializable<app::workflow::ProcessTrackerRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::workflow::ProcessTrackerRecord::get_id,
                &app::workflow::ProcessTrackerRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "name",
                &app::workflow::ProcessTrackerRecord::get_name,
                &app::workflow::ProcessTrackerRecord::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payload",
                &app::workflow::ProcessTrackerRecord::get_payload,
                &app::workflow::ProcessTrackerRecord::set_payload,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::workflow::ProcessTrackerRecord::get_status,
                &app::workflow::ProcessTrackerRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "run_at",
                &app::workflow::ProcessTrackerRecord::get_run_at,
                &app::workflow::ProcessTrackerRecord::set_run_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempts",
                &app::workflow::ProcessTrackerRecord::get_attempts,
                &app::workflow::ProcessTrackerRecord::set_attempts,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "max_attempts",
                &app::workflow::ProcessTrackerRecord::get_max_attempts,
                &app::workflow::ProcessTrackerRecord::set_max_attempts,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "result",
                &app::workflow::ProcessTrackerRecord::get_result,
                &app::workflow::ProcessTrackerRecord::set_result,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error",
                &app::workflow::ProcessTrackerRecord::get_error,
                &app::workflow::ProcessTrackerRecord::set_error,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::workflow::ProcessTrackerRecord::get_created_at,
                &app::workflow::ProcessTrackerRecord::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_at",
                &app::workflow::ProcessTrackerRecord::get_updated_at,
                &app::workflow::ProcessTrackerRecord::set_updated_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "process_trackers";
    }
};

#ifdef CONGELADO_TEST
namespace app::workflow::tests {

using namespace boost::ut;

suite<"app_workflow_process_tracker_record"> process_tracker_record_suite = []
{
    "stores tracker fields and reports its table"_test = []
    {
        ProcessTrackerRecord record{};
        record.set_name("api_key_expiry");
        record.set_payload("key_1");
        record.set_status("PENDING");
        record.set_attempts(0);
        record.set_max_attempts(3);

        expect(record.get_name() == "api_key_expiry");
        expect(record.get_payload() == "key_1");
        expect(record.get_status() == "PENDING");
        expect(record.get_attempts() == 0);
        expect(record.get_max_attempts() == 3);
        expect(serde::Serializable<ProcessTrackerRecord>::table_name() == "process_trackers");
    };
};

} // namespace app::workflow::tests
#endif
