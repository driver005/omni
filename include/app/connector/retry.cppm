module;
export module app_connector:retry;

import std;
import :response;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::connector {

/** @brief Retry decision as data: attempt number and backoff, no sleeps or timers. */
class RetryDecision
{
public:
    RetryDecision() = default;

    void set_retry(bool value) noexcept
    {
        m_retry = value;
    }
    [[nodiscard]] const bool& get_retry() const noexcept
    {
        return m_retry;
    }

    void set_attempt(std::int64_t value) noexcept
    {
        m_attempt = value;
    }
    [[nodiscard]] const std::int64_t& get_attempt() const noexcept
    {
        return m_attempt;
    }

    void set_backoff_ms(std::int64_t value) noexcept
    {
        m_backoff_ms = value;
    }
    [[nodiscard]] const std::int64_t& get_backoff_ms() const noexcept
    {
        return m_backoff_ms;
    }

private:
    bool m_retry{};
    std::int64_t m_attempt{};
    std::int64_t m_backoff_ms{};
};

/** @brief Decides whether a failed connector call should be retried, using data only. */
class RetryPolicy
{
public:
    RetryPolicy() = default;

    RetryPolicy(
        std::int64_t max_attempts,
        std::int64_t base_backoff_ms,
        std::vector<std::string> retryable_statuses
    ) :
        m_max_attempts{max_attempts},
        m_base_backoff_ms{base_backoff_ms},
        m_retryable_statuses{std::move(retryable_statuses)}
    {
    }

    [[nodiscard]] RetryDecision decide(const ConnectorResponse& response, std::int64_t current_attempt) const
    {
        RetryDecision decision{};
        const std::int64_t next_attempt = current_attempt + 1;
        decision.set_attempt(next_attempt);

        const bool status_retryable = is_retryable_status(response.get_status());
        const bool eligible = response.get_retryable() || status_retryable;
        if (!eligible || current_attempt >= m_max_attempts) {
            decision.set_retry(false);
            decision.set_backoff_ms(0);
            return decision;
        }

        decision.set_retry(true);
        std::int64_t backoff = m_base_backoff_ms;
        for (std::int64_t step = 1; step < current_attempt; ++step) {
            backoff *= m_multiplier;
        }
        decision.set_backoff_ms(backoff);
        return decision;
    }

private:
    [[nodiscard]] bool is_retryable_status(std::string_view status) const
    {
        for (const auto& retryable: m_retryable_statuses) {
            if (retryable == status) {
                return true;
            }
        }
        return false;
    }

    std::int64_t m_max_attempts{3};
    std::int64_t m_base_backoff_ms{100};
    std::int64_t m_multiplier{2};
    std::vector<std::string> m_retryable_statuses{"failed", "timeout", "connector_error", "pending"};
};

} // namespace app::connector

#ifdef CONGELADO_TEST
namespace app::connector::tests {
using namespace boost::ut;

suite<"app_connector_retry"> retry_suite = []
{
    "retryable response within the attempt budget is retried with backoff"_test = []
    {
        ConnectorResponse response{};
        response.set_retryable(true);
        const auto decision = RetryPolicy{}.decide(response, 1);
        expect(decision.get_retry());
        expect(decision.get_attempt() == 2);
        expect(decision.get_backoff_ms() == 100);
    };

    "backoff grows with the attempt number"_test = []
    {
        ConnectorResponse response{};
        response.set_retryable(true);
        const auto decision = RetryPolicy{}.decide(response, 2);
        expect(decision.get_retry());
        expect(decision.get_backoff_ms() == 200);
    };

    "terminal response and exhausted budget stop retrying"_test = []
    {
        ConnectorResponse response{};
        response.set_status("authorized");
        expect(!RetryPolicy{}.decide(response, 1).get_retry());

        ConnectorResponse retryable{};
        retryable.set_retryable(true);
        const auto exhausted = RetryPolicy{}.decide(retryable, 3);
        expect(!exhausted.get_retry());
        expect(exhausted.get_backoff_ms() == 0);
    };

    "known retryable status is retried even when the flag is off"_test = []
    {
        ConnectorResponse response{};
        response.set_status("timeout");
        expect(RetryPolicy{}.decide(response, 1).get_retry());
    };
};

} // namespace app::connector::tests
#endif
