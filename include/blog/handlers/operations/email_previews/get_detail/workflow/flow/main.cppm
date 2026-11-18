export module blog_operations:email_previews_get_detail_flow;

import std;
import core_workflow;
import :email_previews_get_detail_step;

export namespace blog::handlers::operations::email_previews::get_detail::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("email_previews_get_detail").step<ExecuteStep>().build();
    }
};

} // namespace
