export module blog_operations:tinybird_tinybird_token_get_flow;

import std;
import core_workflow;
import :tinybird_tinybird_token_get_step;

export namespace blog::handlers::operations::tinybird::tinybird_token_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tinybird_tinybird_token_get").step<ExecuteStep>().build();
    }
};

} // namespace
