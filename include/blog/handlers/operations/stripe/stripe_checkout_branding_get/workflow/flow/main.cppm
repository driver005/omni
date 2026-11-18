export module blog_operations:stripe_stripe_checkout_branding_get_flow;

import std;
import core_workflow;
import :stripe_stripe_checkout_branding_get_step;

export namespace blog::handlers::operations::stripe::stripe_checkout_branding_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("stripe_stripe_checkout_branding_get").step<ExecuteStep>().build();
    }
};

} // namespace
