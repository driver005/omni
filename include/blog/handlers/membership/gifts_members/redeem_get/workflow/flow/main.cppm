export module blog_handlers:gifts_members_redeem_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :gifts_members_redeem_get_step;

export namespace blog::handlers::gifts_members::redeem_get::workflow {

class MainFlow {
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("gifts_members_redeem_get")
            .step<ExecuteStep>()
            .build();
    }
};

} // namespace blog::handlers::gifts_members::redeem_get::workflow
