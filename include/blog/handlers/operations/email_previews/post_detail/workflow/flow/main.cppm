export module blog_operations:email_previews_post_detail_flow;

import std;
import core_workflow;
import :email_previews_post_detail_step;

export namespace blog::handlers::operations::email_previews::post_detail::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("email_previews_post_detail").step<ExecuteStep>().build();
    }
};

} // namespace
