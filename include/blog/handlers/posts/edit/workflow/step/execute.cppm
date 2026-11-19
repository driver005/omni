export module blog_handlers:posts_edit_step;

import std;
import core_workflow;
import interfaces;
import blog_context;
import :mutation;

export namespace blog::handlers::posts::edit::workflow {

class ExecuteStep
{
public:
    using Input=interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"posts_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        blog::api::MutationStore mutations{m_ctx->store()};
        return core::workflow::StepResponse<Output>{mutations.edit("posts", "1", input.body_as_string().empty()?"{}":input.body_as_string()).body()};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* m_ctx;
};

blog::BlogContext* ExecuteStep::m_ctx{nullptr};

} // namespace
