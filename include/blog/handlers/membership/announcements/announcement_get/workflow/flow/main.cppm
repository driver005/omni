export module blog_handlers:announcements_announcement_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :announcements_announcement_get_step;

export namespace blog::handlers::announcements::announcement_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("announcements_announcement_get")
            .step<ExecuteStep>()
            .build();
    }
};

} // namespace blog::handlers::announcements::announcement_get::workflow
