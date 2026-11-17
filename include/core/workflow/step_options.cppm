export module core_workflow:step_options;

import std;

export namespace core::workflow {

/**
 * @brief Per-step behavior, mirroring TransactionStepsDefinition's step options.
 * Fluent instance setters; no static factories.
 */
class StepOptions
{
public:
    [[nodiscard]] int max_retries() const noexcept
    {
        return m_max_retries;
    }

    [[nodiscard]] std::chrono::milliseconds retry_interval() const noexcept
    {
        return m_retry_interval;
    }

    [[nodiscard]] const std::optional<std::chrono::milliseconds>& timeout() const noexcept
    {
        return m_timeout;
    }

    [[nodiscard]] bool async() const noexcept
    {
        return m_async;
    }

    [[nodiscard]] bool compensate_async() const noexcept
    {
        return m_compensate_async;
    }

    [[nodiscard]] bool no_compensation() const noexcept
    {
        return m_no_compensation;
    }

    [[nodiscard]] bool save_response() const noexcept
    {
        return m_save_response;
    }

    [[nodiscard]] bool continue_on_permanent_failure() const noexcept
    {
        return m_continue_on_permanent_failure;
    }

    [[nodiscard]] const std::optional<std::string>& skip_on_permanent_failure() const noexcept
    {
        return m_skip_on_permanent_failure;
    }

    StepOptions& max_retries(int count) noexcept
    {
        m_max_retries = count < 0 ? 0 : count;
        return *this;
    }

    StepOptions& retry_interval(std::chrono::milliseconds interval) noexcept
    {
        m_retry_interval = interval;
        return *this;
    }

    StepOptions& timeout(std::chrono::milliseconds limit) noexcept
    {
        m_timeout = limit;
        return *this;
    }

    StepOptions& async(bool enabled) noexcept
    {
        m_async = enabled;
        return *this;
    }

    StepOptions& compensate_async(bool enabled) noexcept
    {
        m_compensate_async = enabled;
        return *this;
    }

    StepOptions& no_compensation(bool enabled) noexcept
    {
        m_no_compensation = enabled;
        return *this;
    }

    StepOptions& save_response(bool enabled) noexcept
    {
        m_save_response = enabled;
        return *this;
    }

    StepOptions& continue_on_permanent_failure(bool enabled) noexcept
    {
        m_continue_on_permanent_failure = enabled;
        return *this;
    }

    StepOptions& skip_on_permanent_failure(std::string step_name)
    {
        m_skip_on_permanent_failure = std::move(step_name);
        return *this;
    }

private:
    int m_max_retries{0};
    std::chrono::milliseconds m_retry_interval{0};
    std::optional<std::chrono::milliseconds> m_timeout{};
    bool m_async{false};
    bool m_compensate_async{false};
    bool m_no_compensation{false};
    bool m_save_response{true};
    bool m_continue_on_permanent_failure{false};
    std::optional<std::string> m_skip_on_permanent_failure{};
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

suite<"core_workflow_step_options"> core_workflow_step_options_suite = []
{
    "fluent setters apply and clamp negative retries"_test = []
    {
        StepOptions opts;
        opts.max_retries(3)
            .retry_interval(std::chrono::milliseconds{25})
            .timeout(std::chrono::milliseconds{500})
            .async(true)
            .no_compensation(true)
            .save_response(false);
        expect(opts.max_retries() == 3);
        expect(opts.retry_interval() == std::chrono::milliseconds{25});
        expect(opts.timeout().has_value());
        expect(opts.async());
        expect(opts.no_compensation());
        expect(!opts.save_response());
        expect(opts.max_retries(-1) == opts.max_retries());
    };
};

} // namespace core::workflow::tests
#endif
