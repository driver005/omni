module;
export module app_workflow:retry_policy;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::workflow {

/** @brief Data-only backoff policy; delays are values, never sleeps. */
class RetryPolicy
{
public:
    RetryPolicy() = default;

    [[nodiscard]] bool should_retry(std::int64_t attempts) const noexcept
    {
        return m_max_attempts > 0 && attempts < m_max_attempts;
    }

    [[nodiscard]] std::int64_t delay_seconds(std::int64_t attempts) const noexcept
    {
        if (attempts <= 0) {
            return m_backoff_seconds;
        }
        const std::int64_t multiplier = m_multiplier > 0 ? m_multiplier : 1;
        std::int64_t delay = m_backoff_seconds;
        for (std::int64_t step = 1; step < attempts; ++step) {
            if (delay > m_max_delay_seconds / multiplier) {
                return m_max_delay_seconds;
            }
            delay *= multiplier;
        }
        return delay > m_max_delay_seconds ? m_max_delay_seconds : delay;
    }

    std::int64_t m_max_attempts{3};
    std::int64_t m_backoff_seconds{1};
    std::int64_t m_multiplier{2};
    std::int64_t m_max_delay_seconds{60};
};

} // namespace app::workflow

#ifdef CONGELADO_TEST
namespace app::workflow::tests {

using namespace boost::ut;

suite<"app_workflow_retry_policy"> retry_policy_suite = []
{
    "decides retries from attempt count"_test = []
    {
        RetryPolicy policy{};
        expect(policy.should_retry(0));
        expect(policy.should_retry(2));
        expect(!policy.should_retry(3));
    };

    "computes bounded exponential backoff without sleeping"_test = []
    {
        RetryPolicy policy{};
        policy.m_backoff_seconds = 2;
        policy.m_multiplier = 3;
        policy.m_max_delay_seconds = 10;

        expect(policy.delay_seconds(0) == 2);
        expect(policy.delay_seconds(1) == 2);
        expect(policy.delay_seconds(2) == 6);
        expect(policy.delay_seconds(3) == 10);
    };
};

} // namespace app::workflow::tests
#endif
