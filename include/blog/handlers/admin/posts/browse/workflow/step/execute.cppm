export module blog_handlers:posts_browse_step;
import :domain_all;

import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::posts::browse::workflow {

class ExecuteStep
{
public:
    using Input=interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"posts_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& mutations = m_ctx->store();
        std::map<std::string, std::string> params;
        const auto q = std::string{input.get_path()}.find('?');
        if (q != std::string::npos) {
            for (auto pair : std::string_view{std::string{input.get_path()}}.substr(q + 1) | std::views::split('&')) {
                const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
                const auto eq = text.find('=');
                if (eq != std::string::npos) params[text.substr(0, eq)] = text.substr(eq + 1);
            }
        }
        const auto rows = m_ctx->store().json_for("posts", params);
        return core::workflow::StepResponse<Output>{"{\"posts\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}"};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* m_ctx;
};

blog::BlogContext* ExecuteStep::m_ctx{nullptr};

} // namespace
