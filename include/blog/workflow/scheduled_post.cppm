export module blog_workflow:scheduled_post;

import std;
import core_workflow;
import :step_scheduled_post_select_due;
import :step_scheduled_post_publish_post;
import :step_scheduled_post_mark_published;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Scheduled post workflow definition builder. */
class ScheduledPostWorkflow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
                        .name("scheduled_post")
                        .step<SelectDueScheduledPostStep>()
                        .step<PublishPostScheduledPostStep>()
                        .step<MarkPublishedScheduledPostStep>()
                        .build();
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_scheduled_post"> blog_workflow_scheduled_post_suite = []
{
    "builds a valid definition"_test = []
    {
        const auto definition = ScheduledPostWorkflow{}.build();
        expect(definition.valid());
        expect(definition.name == "scheduled_post");
        expect(definition.steps.size() == 3);
    };
};
} // namespace blog::workflow::tests
#endif
