export module blog_handlers:posts_create_step;
import std;
import core_workflow;
import interfaces;
import blog_context;
import :mutation;

export namespace blog::handlers::posts::create::workflow {

class ExecuteStep
{
public:
    using Input=interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"posts_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        blog::api::MutationStore mutations{m_ctx->store()};
        const auto result = mutations.create("posts", input.body_as_string().empty() ? "{}" : input.body_as_string());
        return core::workflow::StepResponse<Output>{{
            {"status", std::to_string(result.status())},
            {"body", result.body()}
        }};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* m_ctx;
};

blog::BlogContext* ExecuteStep::m_ctx{nullptr};

} // namespace
