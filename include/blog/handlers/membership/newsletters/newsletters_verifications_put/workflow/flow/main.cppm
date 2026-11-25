export module blog_handlers:newsletters_newsletters_verifications_put_flow;
export import core_workflow;
import std;
import core_workflow;
import :newsletters_newsletters_verifications_put_step;

export namespace blog::handlers::newsletters::newsletters_verifications_put::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("newsletters_newsletters_verifications_put")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::newsletters::newsletters_verifications_put::workflow
