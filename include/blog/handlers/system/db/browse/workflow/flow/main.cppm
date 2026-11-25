export module blog_handlers:db_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :db_browse_step;

export namespace blog::handlers::db::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("db_browse").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::db::browse::workflow
