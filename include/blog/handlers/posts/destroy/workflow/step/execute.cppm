export module blog_handlers:posts_destroy_step;

import std;
import core_workflow;
import interfaces;
import blog_context;

export namespace blog::handlers::posts::destroy::workflow {

class ExecuteStep
{
public:
    using Input=interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"posts_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& mutations = m_ctx->store();
        return core::workflow::StepResponse<Output>{mutations.destroy("posts", "1").body()};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* m_ctx;
};

blog::BlogContext* ExecuteStep::m_ctx{nullptr};

} // namespace
