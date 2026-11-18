export module blog_handlers_posts_browse:step_parse_query;

import std;
import core_workflow;
import :model_query_input;

export namespace blog::handlers::posts::browse::workflow {

/** @brief Step: normalize raw request path into query parameters. */
class ParseQueryStep
{
public:
    using Input = std::string;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"parse_query"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& path,
        core::workflow::WorkflowContext&
    ) const
    {
        Output out;
        const auto q = path.find('?');
        if (q != std::string::npos) {
            for (auto pair : std::string_view{path}.substr(q + 1) | std::views::split('&')) {
                const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
                const auto eq = text.find('=');
                if (eq != std::string::npos) out[text.substr(0, eq)] = text.substr(eq + 1);
            }
        }
        if (!out.contains("page")) out["page"] = "1";
        if (!out.contains("limit")) out["limit"] = "15";
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace blog::handlers::posts::browse::workflow
