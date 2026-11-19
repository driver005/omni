export module blog_handlers:posts_bulk_edit_step;
import :domain_all;

import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::posts::bulk_edit::workflow {

class ExecuteStep
{
public:
    using Input=interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"posts_bulk_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& mutations = m_ctx->store();
        return core::workflow::StepResponse<Output>{mutations.bulk_edit("posts", input.body_as_string().empty()?"{}":input.body_as_string()).body()};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* m_ctx;
};

blog::BlogContext* ExecuteStep::m_ctx{nullptr};

} // namespace
