export module blog_operations:stripe_stripe_checkout_config_put_step;

import std;
import core_workflow;
import blog_handlers_runtime;
import blog_handlers;
import blog_handlers_middleware;
import blog_dto;
import :stripe_stripe_checkout_config_put_model;
import :stripe_stripe_checkout_config_put_dto;

export namespace blog::handlers::operations::stripe::stripe_checkout_config_put::workflow {

class ExecuteStep
{
public:
    using Input = model::RequestData;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"stripe_stripe_checkout_config_put_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& ctx = blog::handlers::runtime::ServiceRegistry::context();
        blog::api::MutationStore mutation{ctx.store()};
        auto& stripe = blog::handlers::runtime::ServiceRegistry::stripe<blog::api::StripeHandler>();
        Output out;
                out["status"]="200";
        out["body"]=(stripe.configure(input.body), stripe.config());
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace
