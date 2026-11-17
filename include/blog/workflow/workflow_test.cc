import std;
import core_workflow;
import blog_workflow;

int main()
{
    auto workflow = blog::workflow::BlogWorkflowBuilder{}.build();
    if (workflow.workflow_names().size() != 4) {
        return 1;
    }

    const auto automation = workflow.run("automation", std::string{"payload"});
    if (!automation.succeeded()) {
        return 2;
    }
    const auto* published = automation.result_of("automation_publish");
    if (published == nullptr) {
        return 3;
    }

    const auto signup = workflow.run("member_signup", std::string{"ada@example.com"});
    if (!signup.succeeded()) {
        return 4;
    }
    const auto* created = signup.result_of("member_signup_create_member");
    if (created == nullptr) {
        return 5;
    }

    const auto invalid = workflow.run("member_signup", std::string{"not-an-email"});
    if (invalid.succeeded()) {
        return 6;
    }

    return 0;
}
