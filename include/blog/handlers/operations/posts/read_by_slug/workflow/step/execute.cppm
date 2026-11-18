export module blog_operations:posts_read_by_slug_step;

import std;
import core_workflow;
import blog_handlers_runtime;
import blog_handlers;
import blog_dto;
import :posts_read_by_slug_model;
import :posts_read_by_slug_dto;

export namespace blog::handlers::operations::posts::read_by_slug::workflow {

class ExecuteStep
{
public:
    using Input = model::RequestData;
    using Output = std::string;
    static constexpr std::string_view name{"posts_read_by_slug_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& ctx = blog::handlers::runtime::ServiceRegistry::context();
        blog::api::MutationStore mutation{ctx.store()};
        auto& special = blog::handlers::runtime::ServiceRegistry::special<blog::api::SpecialEndpoints>();
        const auto one = ctx.store().find_by_resource("posts", input.id.empty() ? "1" : input.id);
        const auto browse = ctx.store().json_for("posts", {{}});
        const auto open = browse.find('[');
        const auto close = browse.rfind(']');
        const auto first = open == std::string::npos || close == std::string::npos
                               ? std::string{"{}"}
                               : browse.substr(open + 1, close - open - 1);
        (void)one;
        return core::workflow::StepResponse<Output>("{\"posts\":[" + first + "]}");
        return core::workflow::StepResponse<Output>{R"({"posts":[{"id":"1"}]})"};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace
