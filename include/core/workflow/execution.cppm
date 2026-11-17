export module core_workflow:execution;

import std;
import :state;
import :step_outcome;

export namespace core::workflow {

/** @brief The handle a caller gets when a workflow starts. */
class WorkflowExecution
{
public:
    std::string id{};
    std::string workflow_name{};
    std::string transaction_id{};
    std::string run_id{};
    WorkflowState state{WorkflowState::NOT_STARTED};
    std::map<std::string, StepOutcome> step_results{};
    std::vector<std::string> errors{};

    [[nodiscard]] bool has_finished() const noexcept
    {
        return IsWorkflowTerminal{}(state);
    }

    [[nodiscard]] bool succeeded() const noexcept
    {
        return state == WorkflowState::DONE;
    }

    [[nodiscard]] const StepOutcome* result_of(std::string_view step_name) const noexcept
    {
        const auto it = step_results.find(std::string{step_name});
        return it == step_results.end() ? nullptr : &it->second;
    }
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

suite<"core_workflow_execution"> core_workflow_execution_suite = []
{
    "reports terminal states"_test = []
    {
        WorkflowExecution execution;
        execution.state = WorkflowState::INVOKING;
        expect(!execution.has_finished());
        execution.state = WorkflowState::REVERTED;
        expect(execution.has_finished());
        expect(!execution.succeeded());
    };
};

} // namespace core::workflow::tests
#endif
