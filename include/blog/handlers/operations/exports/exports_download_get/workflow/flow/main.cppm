export module blog_operations:exports_exports_download_get_flow;

import std;
import core_workflow;
import :exports_exports_download_get_step;

export namespace blog::handlers::operations::exports::exports_download_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("exports_exports_download_get").step<ExecuteStep>().build();
    }
};

} // namespace
