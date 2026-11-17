export module blog_workflow:automation;

import std;
import core_workflow;
import :step_automation_validate;
import :step_automation_publish;
import :step_automation_notify;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Automation workflow definition builder. */
class AutomationWorkflow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
                        .name("automation")
                        .step<ValidateAutomationStep>()
                        .step<PublishAutomationStep>()
                        .step<NotifyAutomationStep>()
                        .build();
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_automation"> blog_workflow_automation_suite = []
{
    "builds a valid definition"_test = []
    {
        const auto definition = AutomationWorkflow{}.build();
        expect(definition.valid());
        expect(definition.name == "automation");
        expect(definition.steps.size() == 3);
    };
};
} // namespace blog::workflow::tests
#endif
