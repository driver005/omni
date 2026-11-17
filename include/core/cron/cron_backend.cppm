export module core_cron:cron_backend;

import std;
import :cron_job;
import :fire_event;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::cron::detail {

/**
 * @brief Backend contract the facade talks to. Purely internal — concrete
 * engines and this interface never leave the module.
 */
class CronBackend
{
public:
    using TimePoint = std::chrono::system_clock::time_point;
    using FireCallback = std::function<void(std::string_view)>;

    virtual ~CronBackend() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual bool is_open() const noexcept = 0;

    virtual void open() = 0;
    virtual void close() = 0;

    virtual void set_fire_callback(FireCallback callback) = 0;
    virtual void upsert(const CronJob& job) = 0;
    virtual bool remove(std::string_view job_name) = 0;
    [[nodiscard]] virtual std::vector<CronJob> jobs() const = 0;

    /** @brief Fires all jobs due at or before `now`, returning the events. */
    [[nodiscard]] virtual std::vector<FireEvent> poll_due(TimePoint now) = 0;
};

} // namespace core::cron::detail

#ifdef CONGELADO_TEST
namespace core::cron::detail::tests {
using namespace boost::ut;

suite<"core_cron_backend"> core_cron_backend_suite = []
{
    "backend contract is abstract"_test = []
    {
        expect(std::is_abstract_v<CronBackend>);
    };
};

} // namespace core::cron::detail::tests
#endif
