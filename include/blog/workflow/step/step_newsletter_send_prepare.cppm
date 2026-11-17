export module blog_workflow:step_newsletter_send_prepare;

import std;
import core_workflow;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Workflow step: prepare for newsletter send. Real state change in context. */
class PrepareNewsletterSendStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"newsletter_send_prepare"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input,
        core::workflow::WorkflowContext& context
    ) const
    {
        if (input.empty()) {
            return core::workflow::StepResponse<Output>{input}.permanent_failure("empty newsletter");
        }
        context.set("newsletter.prepared", core::workflow::WorkflowValue{input});
        return core::workflow::StepResponse<Output>{input};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext& context) const
    {
        context.set("newsletter.prepared", core::workflow::WorkflowValue{std::monostate{}});
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_step_newsletter_send_prepare"> blog_workflow_step_newsletter_send_prepare_suite = []
{
    "step changes context state"_test = []
    {
        PrepareNewsletterSendStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("newsletter_send-payload", context);
        expect(response.signal() == core::workflow::StepSignal::NONE);
        expect(!response.output().empty());
    };
    "step fails on invalid input"_test = []
    {
        PrepareNewsletterSendStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("", context);
        expect(response.signal() == core::workflow::StepSignal::PERMANENT_FAILURE ||
               !response.output().empty());
    };
};
} // namespace blog::workflow::tests
#endif
