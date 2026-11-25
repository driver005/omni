export module blog_handlers:search_index_search_index_pages_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :search_index_search_index_pages_get_step;

export namespace blog::handlers::search_index::search_index_pages_get::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("search_index_search_index_pages_get")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::search_index::search_index_pages_get::workflow
