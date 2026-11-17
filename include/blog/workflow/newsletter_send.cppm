export module blog_workflow:newsletter_send;

import std;
import core_workflow;
import :step_newsletter_send_prepare;
import :step_newsletter_send_render;
import :step_newsletter_send_dispatch;
import :step_newsletter_send_record;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Newsletter send workflow definition builder. */
class NewsletterSendWorkflow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
                        .name("newsletter_send")
                        .step<PrepareNewsletterSendStep>()
                        .step<RenderNewsletterSendStep>()
                        .step<DispatchNewsletterSendStep>()
                        .step<RecordNewsletterSendStep>()
                        .build();
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_newsletter_send"> blog_workflow_newsletter_send_suite = []
{
    "builds a valid definition"_test = []
    {
        const auto definition = NewsletterSendWorkflow{}.build();
        expect(definition.valid());
        expect(definition.name == "newsletter_send");
        expect(definition.steps.size() == 4);
    };
};
} // namespace blog::workflow::tests
#endif
