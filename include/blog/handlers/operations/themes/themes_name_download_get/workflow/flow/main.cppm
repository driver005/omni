export module blog_operations:themes_themes_name_download_get_flow;

import std;
import core_workflow;
import :themes_themes_name_download_get_step;

export namespace blog::handlers::operations::themes::themes_name_download_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("themes_themes_name_download_get").step<ExecuteStep>().build();
    }
};

} // namespace
