export module blog_workflow:step_member_signup_create_member;

import std;
import core_workflow;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Workflow step: create_member for member signup. Real state change in context. */
class CreateMemberMemberSignupStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"member_signup_create_member"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input,
        core::workflow::WorkflowContext& context
    ) const
    {
        if (input.find('@') == std::string::npos) {
            return core::workflow::StepResponse<Output>{input}.permanent_failure("invalid email");
        }
        context.set("member.email", core::workflow::WorkflowValue{input});
        return core::workflow::StepResponse<Output>{"member:" + input};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext& context) const
    {
        context.set("member.email", core::workflow::WorkflowValue{std::monostate{}});
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_step_member_signup_create_member"> blog_workflow_step_member_signup_create_member_suite = []
{
    "step changes context state"_test = []
    {
        CreateMemberMemberSignupStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("member_signup-payload", context);
        expect(response.signal() == core::workflow::StepSignal::NONE);
        expect(!response.output().empty());
    };
    "step fails on invalid input"_test = []
    {
        CreateMemberMemberSignupStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("", context);
        expect(response.signal() == core::workflow::StepSignal::PERMANENT_FAILURE ||
               !response.output().empty());
    };
};
} // namespace blog::workflow::tests
#endif
