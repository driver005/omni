export module blog_operations:posts_gift_links_edit_step;

import std;
import core_workflow;
import blog_handlers_runtime;
import blog_handlers;
import blog_dto;
import :posts_gift_links_edit_model;
import :posts_gift_links_edit_dto;

export namespace blog::handlers::operations::posts::gift_links_edit::workflow {

class ExecuteStep
{
public:
    using Input = model::RequestData;
    using Output = std::string;
    static constexpr std::string_view name{"posts_gift_links_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& ctx = blog::handlers::runtime::ServiceRegistry::context();
        blog::api::MutationStore mutation{ctx.store()};
        auto& special = blog::handlers::runtime::ServiceRegistry::special<blog::api::SpecialEndpoints>();
        return core::workflow::StepResponse<Output>{(special.gift_links()).body()};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace
