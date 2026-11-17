export module blog_workflow:member_signup;

import std;
import core_workflow;
import :step_member_signup_create_member;
import :step_member_signup_send_welcome;
import :step_member_signup_grant_access;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Member signup workflow definition builder. */
class MemberSignupWorkflow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
                        .name("member_signup")
                        .step<CreateMemberMemberSignupStep>()
                        .step<SendWelcomeMemberSignupStep>()
                        .step<GrantAccessMemberSignupStep>()
                        .build();
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_member_signup"> blog_workflow_member_signup_suite = []
{
    "builds a valid definition"_test = []
    {
        const auto definition = MemberSignupWorkflow{}.build();
        expect(definition.valid());
        expect(definition.name == "member_signup");
        expect(definition.steps.size() == 3);
    };
};
} // namespace blog::workflow::tests
#endif
