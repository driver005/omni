export module blog_jobs:jobs;

import std;
import core_cron;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::jobs {

/** @brief Scheduled-jobs service driven by core_cron. */
class JobService
{
public:
    JobService()
    {
        m_cron.set_fire_callback([](std::string_view) {});
    }

    bool schedule(std::string name, std::string expression)
    {
        const auto job = m_cron.schedule(name, expression);
        return job.has_value();
    }

    [[nodiscard]] std::size_t count() const noexcept
    {
        return m_cron.jobs().size();
    }

private:
    core::cron::Cron m_cron{};
};

} // namespace blog::jobs
#ifdef CONGELADO_TEST
namespace blog::jobs::tests {
using namespace boost::ut;
suite<"blog_jobs_jobs"> blog_jobs_jobs_suite = []
{
    "schedules jobs"_test = []
    {
        JobService jobs;
        expect(jobs.schedule("nightly", "0 2 * * *"));
        expect(jobs.count() == 1);
    };
};
} // namespace blog::jobs::tests
#endif
