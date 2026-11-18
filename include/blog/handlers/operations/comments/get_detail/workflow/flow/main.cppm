export module blog_operations:comments_get_detail_flow;

import std;
import core_workflow;
import :comments_get_detail_step;

export namespace blog::handlers::operations::comments::get_detail::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("comments_get_detail").step<ExecuteStep>().build();
    }
};

} // namespace
