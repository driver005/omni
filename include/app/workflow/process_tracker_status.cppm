module;
export module app_workflow:process_tracker_status;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::workflow {

/** @brief Canonical status strings for process-tracker records, as instance methods. */
class ProcessTrackerStatus
{
public:
    [[nodiscard]] std::string_view pending() const noexcept
    {
        return "PENDING";
    }

    [[nodiscard]] std::string_view running() const noexcept
    {
        return "RUNNING";
    }

    [[nodiscard]] std::string_view retrying() const noexcept
    {
        return "RETRYING";
    }

    [[nodiscard]] std::string_view completed() const noexcept
    {
        return "COMPLETED";
    }

    [[nodiscard]] std::string_view failed() const noexcept
    {
        return "FAILED";
    }

    [[nodiscard]] std::string_view cancelled() const noexcept
    {
        return "CANCELLED";
    }
};

} // namespace app::workflow

#ifdef CONGELADO_TEST
namespace app::workflow::tests {

using namespace boost::ut;

suite<"app_workflow_process_tracker_status"> process_tracker_status_suite = []
{
    "exposes stable wire names"_test = []
    {
        const ProcessTrackerStatus status{};
        expect(status.pending() == "PENDING");
        expect(status.retrying() == "RETRYING");
        expect(status.completed() == "COMPLETED");
    };
};

} // namespace app::workflow::tests
#endif
