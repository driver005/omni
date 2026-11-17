export module core_workflow:step_outcome;

import std;
import :value;
import :state;

export namespace core::workflow {

/** @brief Type-erased result of one step invocation, stored in a checkpoint. */
struct StepOutcome
{
    StepSignal signal{StepSignal::NONE};
    WorkflowValue output{std::monostate{}};
    WorkflowValue compensate_input{std::monostate{}};
    std::string message{};
    std::any typed_output{};
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

suite<"core_workflow_step_outcome"> core_workflow_step_outcome_suite = []
{
    "defaults to an empty successful outcome"_test = []
    {
        const StepOutcome outcome;
        expect(outcome.signal == StepSignal::NONE);
        expect(std::holds_alternative<std::monostate>(outcome.output));
        expect(outcome.message.empty());
    };
};

} // namespace core::workflow::tests
#endif
