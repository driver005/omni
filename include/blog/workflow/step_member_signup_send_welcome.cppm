export module blog_workflow:step_member_signup_send_welcome;

import std;
import core_workflow;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Workflow step: send_welcome for member signup. Real state change in context. */
class SendWelcomeMemberSignupStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"member_signup_send_welcome"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input,
        core::workflow::WorkflowContext& context
    ) const
    {
        const auto email = context.get("member.email");
        if (std::holds_alternative<std::monostate>(email)) {
            return core::workflow::StepResponse<Output>{input}.permanent_failure("no member email");
        }
        context.set("member.welcomed", core::workflow::WorkflowValue{true});
        return core::workflow::StepResponse<Output>{"welcome:" + input};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext& context) const
    {
        context.set("member.welcomed", core::workflow::WorkflowValue{std::monostate{}});
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_step_member_signup_send_welcome"> blog_workflow_step_member_signup_send_welcome_suite = []
{
    "step changes context state"_test = []
    {
        SendWelcomeMemberSignupStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("member_signup-payload", context);
        expect(response.signal() == core::workflow::StepSignal::NONE);
        expect(!response.output().empty());
    };
    "step fails on invalid input"_test = []
    {
        SendWelcomeMemberSignupStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("", context);
        expect(response.signal() == core::workflow::StepSignal::PERMANENT_FAILURE ||
               !response.output().empty());
    };
};
} // namespace blog::workflow::tests
#endif
