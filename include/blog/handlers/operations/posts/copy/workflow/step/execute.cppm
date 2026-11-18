export module blog_operations:posts_copy_step;

import std;
import core_workflow;
import blog_handlers_runtime;
import blog_handlers;
import blog_dto;
import :posts_copy_model;
import :posts_copy_dto;

export namespace blog::handlers::operations::posts::copy::workflow {

class ExecuteStep
{
public:
    using Input = model::RequestData;
    using Output = std::string;
    static constexpr std::string_view name{"posts_copy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& ctx = blog::handlers::runtime::ServiceRegistry::context();
        blog::api::MutationStore mutation{ctx.store()};
        auto& special = blog::handlers::runtime::ServiceRegistry::special<blog::api::SpecialEndpoints>();
        return core::workflow::StepResponse<Output>{(mutation.create("posts", input.body)).body()};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace
