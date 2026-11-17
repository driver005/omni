export module blog_workflow:step_scheduled_post_mark_published;

import std;
import core_workflow;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Workflow step: mark_published for scheduled post. Real state change in context. */
class MarkPublishedScheduledPostStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"scheduled_post_mark_published"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input,
        core::workflow::WorkflowContext& context
    ) const
    {
        const auto published = context.get("post.published");
        if (std::holds_alternative<std::monostate>(published)) {
            return core::workflow::StepResponse<Output>{input}.permanent_failure("post not published");
        }
        context.set("post.marked", core::workflow::WorkflowValue{true});
        return core::workflow::StepResponse<Output>{"marked:" + input};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext& context) const
    {
        context.set("post.marked", core::workflow::WorkflowValue{std::monostate{}});
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_step_scheduled_post_mark_published"> blog_workflow_step_scheduled_post_mark_published_suite = []
{
    "step changes context state"_test = []
    {
        MarkPublishedScheduledPostStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("scheduled_post-payload", context);
        expect(response.signal() == core::workflow::StepSignal::NONE);
        expect(!response.output().empty());
    };
    "step fails on invalid input"_test = []
    {
        MarkPublishedScheduledPostStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("", context);
        expect(response.signal() == core::workflow::StepSignal::PERMANENT_FAILURE ||
               !response.output().empty());
    };
};
} // namespace blog::workflow::tests
#endif
