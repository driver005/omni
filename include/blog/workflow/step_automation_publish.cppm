export module blog_workflow:step_automation_publish;

import std;
import core_workflow;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Workflow step: publish for automation. Real state change in context. */
class PublishAutomationStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"automation_publish"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input,
        core::workflow::WorkflowContext& context
    ) const
    {
        const auto valid = context.get("automation.valid");
        if (std::holds_alternative<std::monostate>(valid)) {
            return core::workflow::StepResponse<Output>{input}.permanent_failure(
                "automation not validated"
            );
        }
        const std::string result = "published:" + input;
        context.set("automation.published", core::workflow::WorkflowValue{result});
        return core::workflow::StepResponse<Output>{result};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext& context) const
    {
        context.set("automation.published", core::workflow::WorkflowValue{std::monostate{}});
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_step_automation_publish"> blog_workflow_step_automation_publish_suite = []
{
    "step changes context state"_test = []
    {
        PublishAutomationStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("automation-payload", context);
        expect(response.signal() == core::workflow::StepSignal::NONE);
        expect(!response.output().empty());
    };
    "step fails on invalid input"_test = []
    {
        PublishAutomationStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("", context);
        expect(response.signal() == core::workflow::StepSignal::PERMANENT_FAILURE ||
               !response.output().empty());
    };
};
} // namespace blog::workflow::tests
#endif
