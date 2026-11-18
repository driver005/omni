export module blog_operations:slugs_get_detail_flow;

import std;
import core_workflow;
import :slugs_get_detail_step;

export namespace blog::handlers::operations::slugs::get_detail::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("slugs_get_detail").step<ExecuteStep>().build();
    }
};

} // namespace
