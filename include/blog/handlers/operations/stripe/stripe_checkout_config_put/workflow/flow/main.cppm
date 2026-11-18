export module blog_operations:stripe_stripe_checkout_config_put_flow;

import std;
import core_workflow;
import :stripe_stripe_checkout_config_put_step;

export namespace blog::handlers::operations::stripe::stripe_checkout_config_put::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("stripe_stripe_checkout_config_put").step<ExecuteStep>().build();
    }
};

} // namespace
