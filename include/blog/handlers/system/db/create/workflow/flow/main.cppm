export module blog_handlers:db_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :db_create_step;

export namespace blog::handlers::db::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("db_create").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::db::create::workflow
