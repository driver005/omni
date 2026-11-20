export module blog_handlers:posts_browse_step;
import :domain_all;
import :posts_support;

import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::posts::browse::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"posts_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::posts::support;
        const bool content = is_content(const_cast<Input&>(input));
        const std::span<const std::string_view> allowed = content
            ? std::span<const std::string_view>{kContentBrowseOptions}
            : std::span<const std::string_view>{kAdminBrowseOptions};
        const auto parsed = parse(input.get_path(), allowed, false, content);
        if (!parsed.error.empty()) {
            return core::workflow::StepResponse<Output>{{{"status", parsed.status},
                {"body", parsed.error}}};
        }
        if (const auto error = validate_options(parsed.query, content)) {
            return core::workflow::StepResponse<Output>{{{"status", "422"}, {"body", *error}}};
        }
        return core::workflow::StepResponse<Output>{{{"status", "200"},
            {"body", browse_body(*s_ctx, parsed.query, content)}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace
