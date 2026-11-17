export module blog_workflow:registry;

import std;
import core_workflow;
import :automation;
import :scheduled_post;
import :newsletter_send;
import :member_signup;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Registers every blog workflow into a core_workflow facade. */
class BlogWorkflowRegistry
{
public:
    void register_all(core::workflow::Workflow& workflow) const
    {
        workflow.register_workflow(AutomationWorkflow{}.build());
        workflow.register_workflow(ScheduledPostWorkflow{}.build());
        workflow.register_workflow(NewsletterSendWorkflow{}.build());
        workflow.register_workflow(MemberSignupWorkflow{}.build());
    }

    [[nodiscard]] std::size_t workflow_count() const noexcept
    {
        return 4;
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_registry"> blog_workflow_registry_suite = []
{
    "registers all workflows"_test = []
    {
        core::workflow::Workflow workflow;
        BlogWorkflowRegistry{}.register_all(workflow);
        expect(workflow.workflow_names().size() == 4);
        expect(BlogWorkflowRegistry{}.workflow_count() == 4);
    };
};
} // namespace blog::workflow::tests
#endif
