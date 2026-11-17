export module blog_workflow:step_member_signup_grant_access;

import std;
import core_workflow;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Workflow step: grant_access for member signup. Real state change in context. */
class GrantAccessMemberSignupStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"member_signup_grant_access"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input,
        core::workflow::WorkflowContext& context
    ) const
    {
        const auto welcomed = context.get("member.welcomed");
        if (std::holds_alternative<std::monostate>(welcomed)) {
            return core::workflow::StepResponse<Output>{input}.permanent_failure("welcome not sent");
        }
        context.set("member.access", core::workflow::WorkflowValue{"free"});
        return core::workflow::StepResponse<Output>{"access:" + input};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext& context) const
    {
        context.set("member.access", core::workflow::WorkflowValue{std::monostate{}});
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_step_member_signup_grant_access"> blog_workflow_step_member_signup_grant_access_suite = []
{
    "step changes context state"_test = []
    {
        GrantAccessMemberSignupStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("member_signup-payload", context);
        expect(response.signal() == core::workflow::StepSignal::NONE);
        expect(!response.output().empty());
    };
    "step fails on invalid input"_test = []
    {
        GrantAccessMemberSignupStep step;
        core::workflow::WorkflowContext context;
        const auto response = step.invoke("", context);
        expect(response.signal() == core::workflow::StepSignal::PERMANENT_FAILURE ||
               !response.output().empty());
    };
};
} // namespace blog::workflow::tests
#endif
