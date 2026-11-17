export module core_workflow:step_response;

import std;
import :value;
import :state;

export namespace core::workflow {

/**
 * @brief What a step hands back: its output, the input its compensation needs,
 * and an optional control-flow signal (permanent failure / skip).
 *
 * Mirrors Medusa's `StepResponse(output, compensateInput?)`, `permanentFailure()`
 * and `skip()`. Compensate input defaults to the output.
 */
template<typename Out>
class StepResponse
{
public:
    using Output = Out;

    StepResponse() = default;

    explicit StepResponse(Out output) :
        m_output{std::move(output)},
        m_compensate_input{ToWorkflowValue{}(m_output)}
    {
    }

    StepResponse(Out output, WorkflowValue compensate_input) :
        m_output{std::move(output)},
        m_compensate_input{std::move(compensate_input)}
    {
    }

    [[nodiscard]] const Out& output() const noexcept
    {
        return m_output;
    }

    [[nodiscard]] WorkflowValue to_value() const
    {
        return ToWorkflowValue{}(m_output);
    }

    [[nodiscard]] std::any typed_output() const
    {
        return std::any{m_output};
    }

    [[nodiscard]] const WorkflowValue& compensate_input() const noexcept
    {
        return m_compensate_input;
    }

    [[nodiscard]] StepSignal signal() const noexcept
    {
        return m_signal;
    }

    [[nodiscard]] const std::string& message() const noexcept
    {
        return m_message;
    }

    StepResponse& with_compensate_input(WorkflowValue compensate_input)
    {
        m_compensate_input = std::move(compensate_input);
        return *this;
    }

    StepResponse& permanent_failure(std::string message, WorkflowValue compensate_input = {})
    {
        m_signal = StepSignal::PERMANENT_FAILURE;
        m_message = std::move(message);
        m_compensate_input = std::move(compensate_input);
        return *this;
    }

    StepResponse& skip()
    {
        m_signal = StepSignal::SKIP;
        return *this;
    }

private:
    Out m_output{};
    WorkflowValue m_compensate_input{};
    StepSignal m_signal{StepSignal::NONE};
    std::string m_message{};
};

/** @brief Void-output steps still carry compensate input and signals. */
template<>
class StepResponse<void>
{
public:
    using Output = void;

    StepResponse() = default;

    explicit StepResponse(WorkflowValue compensate_input) :
        m_compensate_input{std::move(compensate_input)}
    {
    }

    [[nodiscard]] WorkflowValue to_value() const
    {
        return std::monostate{};
    }

    [[nodiscard]] std::any typed_output() const
    {
        return std::any{};
    }

    [[nodiscard]] const WorkflowValue& compensate_input() const noexcept
    {
        return m_compensate_input;
    }

    [[nodiscard]] StepSignal signal() const noexcept
    {
        return m_signal;
    }

    [[nodiscard]] const std::string& message() const noexcept
    {
        return m_message;
    }

    StepResponse& with_compensate_input(WorkflowValue compensate_input)
    {
        m_compensate_input = std::move(compensate_input);
        return *this;
    }

    StepResponse& permanent_failure(std::string message, WorkflowValue compensate_input = {})
    {
        m_signal = StepSignal::PERMANENT_FAILURE;
        m_message = std::move(message);
        m_compensate_input = std::move(compensate_input);
        return *this;
    }

    StepResponse& skip()
    {
        m_signal = StepSignal::SKIP;
        return *this;
    }

private:
    WorkflowValue m_compensate_input{};
    StepSignal m_signal{StepSignal::NONE};
    std::string m_message{};
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

suite<"core_workflow_step_response"> core_workflow_step_response_suite = []
{
    "compensate input defaults to output, can be overridden"_test = []
    {
        const StepResponse<std::string> response{"brand_01"};
        expect(std::get<std::string>(response.compensate_input()) == "brand_01");
        const auto overridden = StepResponse<std::string>{"brand_01"}.with_compensate_input(
            WorkflowValue{std::string{"backup_id"}});
        expect(std::get<std::string>(overridden.compensate_input()) == "backup_id");
    };

    "sentinels are carried as signals"_test = []
    {
        const auto failed = StepResponse<std::string>{"x"}.permanent_failure("bad input");
        expect(failed.signal() == StepSignal::PERMANENT_FAILURE);
        expect(failed.message() == "bad input");
        const auto skipped = StepResponse<void>{}.skip();
        expect(skipped.signal() == StepSignal::SKIP);
    };
};

} // namespace core::workflow::tests
#endif
