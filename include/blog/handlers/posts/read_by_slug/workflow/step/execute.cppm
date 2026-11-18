export module blog_handlers:posts_read_by_slug_step;

import std;
import core_workflow;
import blog_context;
import :mutation;
import :posts_read_by_slug_model;
import :posts_read_by_slug_dto;

export namespace blog::handlers::posts::read_by_slug::workflow {

class ExecuteStep
{
public:
    using Input = model::Request;
    using Output = std::string;
    static constexpr std::string_view name{"posts_read_by_slug_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        blog::api::MutationStore mutations{m_ctx->store()};
        return core::workflow::StepResponse<Output>{"{\"posts\":[]}"};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* m_ctx;
};

blog::BlogContext* ExecuteStep::m_ctx{nullptr};

} // namespace
