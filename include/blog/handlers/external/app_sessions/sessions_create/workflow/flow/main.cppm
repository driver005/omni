export module blog_handlers:app_sessions_sessions_create_flow;
export import core_workflow;
import :domain_all;
import std;
import core_workflow;
import :app_sessions_sessions_create_step;

export namespace blog::handlers::app_sessions::sessions_create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("app_sessions_sessions_create")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::app_sessions::sessions_create::workflow
