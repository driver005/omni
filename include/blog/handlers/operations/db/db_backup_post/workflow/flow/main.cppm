export module blog_operations:db_db_backup_post_flow;

import std;
import core_workflow;
import :db_db_backup_post_step;

export namespace blog::handlers::operations::db::db_backup_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("db_db_backup_post").step<ExecuteStep>().build();
    }
};

} // namespace
