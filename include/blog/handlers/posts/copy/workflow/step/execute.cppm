export module blog_handlers:posts_copy_step;

import std;
import core_workflow;
import blog_context;
import :mutation;
import :posts_copy_model;
import :posts_copy_dto;

export namespace blog::handlers::posts::copy::workflow {

class ExecuteStep
{
public:
    using Input = model::Request;
    using Output = std::string;
    static constexpr std::string_view name{"posts_copy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        blog::api::MutationStore mutations{m_ctx->store()};
        return core::workflow::StepResponse<Output>{mutations.create("posts", input.body.empty()?"{}":input.body).body()};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* m_ctx;
};

blog::BlogContext* ExecuteStep::m_ctx{nullptr};

} // namespace
