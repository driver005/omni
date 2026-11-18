export module blog_operations:slugs_get_detail_step;

import std;
import core_workflow;
import blog_handlers_runtime;
import blog_handlers;
import blog_handlers_middleware;
import blog_dto;
import :slugs_get_detail_model;
import :slugs_get_detail_dto;

export namespace blog::handlers::operations::slugs::get_detail::workflow {

class ExecuteStep
{
public:
    using Input = model::RequestData;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"slugs_get_detail_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& ctx = blog::handlers::runtime::ServiceRegistry::context();
        blog::api::MutationStore mutation{ctx.store()};
        
        Output out;
                const auto one = ctx.store().find_by_resource("slugs", input.id);
        if (one) {
            out["status"]="200";
            out["body"]="{\"slugs\":[" + *one + "]}";
        } else {
            const auto browse = ctx.store().json_for("slugs", {});
            const auto open = browse.find('[');
            const auto close = browse.rfind(']');
            const auto first = open == std::string::npos || close == std::string::npos || close <= open
                                   ? std::string{"{}"}
                                   : browse.substr(open + 1, close - open - 1);
            out["status"]="200";
            out["body"]="{\"slugs\":[" + first + "]}";
        }
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace
