export module blog_operations:db_create_flow;

import std;
import core_workflow;
import :db_create_step;

export namespace blog::handlers::operations::db::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("db_create").step<ExecuteStep>().build();
    }
};

} // namespace
