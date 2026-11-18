export module blog_operations:integrations_read_by_id_step;

import std;
import core_workflow;
import blog_handlers_runtime;
import blog_handlers;
import blog_handlers_middleware;
import blog_dto;
import :integrations_read_by_id_model;
import :integrations_read_by_id_dto;

export namespace blog::handlers::operations::integrations::read_by_id::workflow {

class ExecuteStep
{
public:
    using Input = model::RequestData;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"integrations_read_by_id_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& ctx = blog::handlers::runtime::ServiceRegistry::context();
        blog::api::MutationStore mutation{ctx.store()};
        
        Output out;
                const auto one = ctx.store().find_by_resource("integrations", input.id);
        if (one) {
            out["status"]="200";
            out["body"]="{\"integrations\":[" + *one + "]}";
        } else {
            const auto browse = ctx.store().json_for("integrations", {});
            const auto open = browse.find('[');
            const auto close = browse.rfind(']');
            const auto first = open == std::string::npos || close == std::string::npos || close <= open
                                   ? std::string{"{}"}
                                   : browse.substr(open + 1, close - open - 1);
            out["status"]="200";
            out["body"]="{\"integrations\":[" + first + "]}";
        }
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace
