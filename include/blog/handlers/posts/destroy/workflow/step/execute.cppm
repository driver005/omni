export module blog_handlers:posts_destroy_step;

import std;
import core_workflow;
import blog_context;
import :mutation;
import :posts_destroy_model;
import :posts_destroy_dto;

export namespace blog::handlers::posts::destroy::workflow {

class ExecuteStep
{
public:
    using Input = model::Request;
    using Output = std::string;
    static constexpr std::string_view name{"posts_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        blog::api::MutationStore mutations{m_ctx->store()};
        return core::workflow::StepResponse<Output>{mutations.destroy("posts", "1").body()};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* m_ctx;
};

blog::BlogContext* ExecuteStep::m_ctx{nullptr};

} // namespace
