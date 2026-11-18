export module blog_operations:themes_themes_upload_post_flow;

import std;
import core_workflow;
import :themes_themes_upload_post_step;

export namespace blog::handlers::operations::themes::themes_upload_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("themes_themes_upload_post").step<ExecuteStep>().build();
    }
};

} // namespace
