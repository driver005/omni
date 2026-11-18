export module blog_operations:posts_bulk_destroy_step;

import std;
import core_workflow;
import blog_handlers_runtime;
import blog_handlers;
import blog_dto;
import :posts_bulk_destroy_model;
import :posts_bulk_destroy_dto;

export namespace blog::handlers::operations::posts::bulk_destroy::workflow {

class ExecuteStep
{
public:
    using Input = model::RequestData;
    using Output = std::string;
    static constexpr std::string_view name{"posts_bulk_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& ctx = blog::handlers::runtime::ServiceRegistry::context();
        blog::api::MutationStore mutation{ctx.store()};
        auto& special = blog::handlers::runtime::ServiceRegistry::special<blog::api::SpecialEndpoints>();
        return core::workflow::StepResponse<Output>{(mutation.bulk_destroy("posts", input.body)).body()};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace
