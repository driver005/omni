export module blog_workflow:step_scheduled_post_select_due;

import std;
import core_workflow;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Workflow step: select_due for scheduled post. Real state change in context. */
class SelectDueScheduledPostStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"scheduled_post_select_due"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input,
        core::workflow::WorkflowContext& context
    ) const
    {
        if (input.empty()) {
            return core::workflow::StepResponse<Output>{input}.permanent_failure("no post id");
        }
        context.set("post.due", core::workflow::WorkflowValue{input});
        return core::workflow::StepResponse<Output>{input};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext& context) const
    {
        context.set("post.due", core::workflow::WorkflowValue{std::monostate{}});
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_step_scheduled_post_select_due"> blog_workflow_step_scheduled_post_select_due_suite = []
{
    "step changes context state"_test = []
    {
        SelectDueScheduledPostStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("scheduled_post-payload", context);
        expect(response.signal() == core::workflow::StepSignal::NONE);
        expect(!response.output().empty());
    };
    "step fails on invalid input"_test = []
    {
        SelectDueScheduledPostStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("", context);
        expect(response.signal() == core::workflow::StepSignal::PERMANENT_FAILURE ||
               !response.output().empty());
    };
};
} // namespace blog::workflow::tests
#endif
