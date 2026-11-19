export module blog_handlers:posts_read_by_id_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::posts::read_by_id::workflow {

class ExecuteStep
{
public:
    using Input=interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"posts_read_by_id_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto path = std::string_view{std::string{input.get_path()}}.substr(0, std::string{input.get_path()}.find('?'));
        while (path.size() > 1 && path.back() == '/') path.remove_suffix(1);
        const auto slash = path.find_last_of('/');
        const std::string id{slash == std::string_view::npos ? path : path.substr(slash + 1)};
        const auto entity = m_ctx->store().find_by_resource("posts", id);
        if (!entity) {
            return core::workflow::StepResponse<Output>{{{"status", "404"}, {"body", R"({"posts":[]})"}}};
        }
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", "{\"posts\":[" + *entity + "]}"}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* m_ctx;
};

blog::BlogContext* ExecuteStep::m_ctx{nullptr};

} // namespace
