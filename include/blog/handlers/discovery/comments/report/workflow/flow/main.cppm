export module blog_handlers:comments_report_flow;
export import core_workflow;
import :domain_all;
import std;
import core_workflow;
import :comments_report_step;

export namespace blog::handlers::comments::report::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("comments_report")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::comments::report::workflow
