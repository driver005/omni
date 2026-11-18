export module blog_operations:redirects_redirects_upload_post_flow;

import std;
import core_workflow;
import :redirects_redirects_upload_post_step;

export namespace blog::handlers::operations::redirects::redirects_upload_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("redirects_redirects_upload_post").step<ExecuteStep>().build();
    }
};

} // namespace
