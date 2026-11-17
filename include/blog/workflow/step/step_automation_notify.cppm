export module blog_workflow:step_automation_notify;

import std;
import core_workflow;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Workflow step: notify for automation. Real state change in context. */
class NotifyAutomationStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"automation_notify"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input,
        core::workflow::WorkflowContext& context
    ) const
    {
        const auto published = context.get("automation.published");
        if (std::holds_alternative<std::monostate>(published)) {
            return core::workflow::StepResponse<Output>{input}.permanent_failure(
                "nothing published to notify"
            );
        }
        context.set("automation.notified", core::workflow::WorkflowValue{true});
        return core::workflow::StepResponse<Output>{"notified:" + input};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext& context) const
    {
        context.set("automation.notified", core::workflow::WorkflowValue{std::monostate{}});
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_step_automation_notify"> blog_workflow_step_automation_notify_suite = []
{
    "step changes context state"_test = []
    {
        NotifyAutomationStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("automation-payload", context);
        expect(response.signal() == core::workflow::StepSignal::NONE);
        expect(!response.output().empty());
    };
    "step fails on invalid input"_test = []
    {
        NotifyAutomationStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("", context);
        expect(response.signal() == core::workflow::StepSignal::PERMANENT_FAILURE ||
               !response.output().empty());
    };
};
} // namespace blog::workflow::tests
#endif
