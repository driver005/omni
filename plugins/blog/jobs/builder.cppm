export module blog_jobs:builder;
import std;
import :jobs;
export namespace blog::jobs {
class JobServiceBuilder
{
public:
    [[nodiscard]] JobService build() const { return JobService{}; }
};
} // namespace blog::jobs
