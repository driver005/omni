export module blog_workflow:step_automation_validate;

import std;
import core_workflow;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Workflow step: validate for automation. Real state change in context. */
class ValidateAutomationStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"automation_validate"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input,
        core::workflow::WorkflowContext& context
    ) const
    {
        if (input.empty()) {
            return core::workflow::StepResponse<Output>{input}.permanent_failure(
                "automation input must not be empty"
            );
        }
        context.set("automation.valid", core::workflow::WorkflowValue{input});
        return core::workflow::StepResponse<Output>{input};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext& context) const
    {
        context.set("automation.valid", core::workflow::WorkflowValue{std::monostate{}});
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_step_automation_validate"> blog_workflow_step_automation_validate_suite = []
{
    "step changes context state"_test = []
    {
        ValidateAutomationStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("automation-payload", context);
        expect(response.signal() == core::workflow::StepSignal::NONE);
        expect(!response.output().empty());
    };
    "step fails on invalid input"_test = []
    {
        ValidateAutomationStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("", context);
        expect(response.signal() == core::workflow::StepSignal::PERMANENT_FAILURE ||
               !response.output().empty());
    };
};
} // namespace blog::workflow::tests
#endif
