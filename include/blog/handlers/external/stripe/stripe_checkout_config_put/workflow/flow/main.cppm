export module blog_handlers:stripe_stripe_checkout_config_put_flow;
export import core_workflow;
import std;
import core_workflow;
import :stripe_stripe_checkout_config_put_step;

export namespace blog::handlers::stripe::stripe_checkout_config_put::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("stripe_stripe_checkout_config_put")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::stripe::stripe_checkout_config_put::workflow
