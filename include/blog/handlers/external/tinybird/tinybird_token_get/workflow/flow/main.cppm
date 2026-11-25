export module blog_handlers:tinybird_tinybird_token_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :tinybird_tinybird_token_get_step;

export namespace blog::handlers::tinybird::tinybird_token_get::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("tinybird_tinybird_token_get")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::tinybird::tinybird_token_get::workflow
