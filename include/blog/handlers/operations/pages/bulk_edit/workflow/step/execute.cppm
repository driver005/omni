export module blog_operations:pages_bulk_edit_step;

import std;
import core_workflow;
import blog_handlers_runtime;
import blog_handlers;
import blog_handlers_middleware;
import blog_dto;
import :pages_bulk_edit_model;
import :pages_bulk_edit_dto;

export namespace blog::handlers::operations::pages::bulk_edit::workflow {

class ExecuteStep
{
public:
    using Input = model::RequestData;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"pages_bulk_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& ctx = blog::handlers::runtime::ServiceRegistry::context();
        blog::api::MutationStore mutation{ctx.store()};
        
        Output out;
                const auto result = mutation.bulk_edit("pages", input.body.empty() ? "{}" : input.body);
        out["status"]=std::to_string(result.status());
        out["body"]=result.body();
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace
