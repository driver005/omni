export module blog_operations:newsletters_newsletters_verifications_put_flow;

import std;
import core_workflow;
import :newsletters_newsletters_verifications_put_step;

export namespace blog::handlers::operations::newsletters::newsletters_verifications_put::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("newsletters_newsletters_verifications_put").step<ExecuteStep>().build();
    }
};

} // namespace
