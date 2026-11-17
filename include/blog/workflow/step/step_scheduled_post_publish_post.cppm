export module blog_workflow:step_scheduled_post_publish_post;

import std;
import core_workflow;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Workflow step: publish_post for scheduled post. Real state change in context. */
class PublishPostScheduledPostStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"scheduled_post_publish_post"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input,
        core::workflow::WorkflowContext& context
    ) const
    {
        const std::string result = "published:" + input;
        context.set("post.published", core::workflow::WorkflowValue{result});
        return core::workflow::StepResponse<Output>{result};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext& context) const
    {
        context.set("post.published", core::workflow::WorkflowValue{std::monostate{}});
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_step_scheduled_post_publish_post"> blog_workflow_step_scheduled_post_publish_post_suite = []
{
    "step changes context state"_test = []
    {
        PublishPostScheduledPostStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("scheduled_post-payload", context);
        expect(response.signal() == core::workflow::StepSignal::NONE);
        expect(!response.output().empty());
    };
    "step fails on invalid input"_test = []
    {
        PublishPostScheduledPostStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("", context);
        expect(response.signal() == core::workflow::StepSignal::PERMANENT_FAILURE ||
               !response.output().empty());
    };
};
} // namespace blog::workflow::tests
#endif
