export module blog_operations:members_members_upload_post_flow;

import std;
import core_workflow;
import :members_members_upload_post_step;

export namespace blog::handlers::operations::members::members_upload_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_members_upload_post").step<ExecuteStep>().build();
    }
};

} // namespace
