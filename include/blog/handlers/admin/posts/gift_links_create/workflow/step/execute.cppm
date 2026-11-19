export module blog_handlers:posts_gift_links_create_step;
import :domain_all;

import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::posts::gift_links_create::workflow {

class ExecuteStep
{
public:
    using Input=interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"posts_gift_links_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& mutations = m_ctx->store();
        return core::workflow::StepResponse<Output>{R"({"gift_links":[{"id":"1","url":"https://ghost.local/link"}]})"};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* m_ctx;
};

blog::BlogContext* ExecuteStep::m_ctx{nullptr};

} // namespace
