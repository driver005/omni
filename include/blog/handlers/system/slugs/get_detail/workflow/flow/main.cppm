export module blog_handlers:slugs_get_detail_flow;
export import core_workflow;
import std;
import core_workflow;
import :slugs_get_detail_step;

export namespace blog::handlers::slugs::get_detail::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("slugs_generate").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::slugs::get_detail::workflow
