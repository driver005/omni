export module blog_operations:members_members_id_commenting_disable_post_flow;

import std;
import core_workflow;
import :members_members_id_commenting_disable_post_step;

export namespace blog::handlers::operations::members::members_id_commenting_disable_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_members_id_commenting_disable_post").step<ExecuteStep>().build();
    }
};

} // namespace
