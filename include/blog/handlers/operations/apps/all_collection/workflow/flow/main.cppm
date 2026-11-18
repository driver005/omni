export module blog_operations:apps_all_collection_flow;

import std;
import core_workflow;
import :apps_all_collection_step;

export namespace blog::handlers::operations::apps::all_collection::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("apps_all_collection").step<ExecuteStep>().build();
    }
};

} // namespace
