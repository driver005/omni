export module blog_operations:newsletters_newsletters_verifications_put_step;

import std;
import core_workflow;
import blog_handlers_runtime;
import blog_handlers;
import blog_handlers_middleware;
import blog_dto;
import :newsletters_newsletters_verifications_put_model;
import :newsletters_newsletters_verifications_put_dto;

export namespace blog::handlers::operations::newsletters::newsletters_verifications_put::workflow {

class ExecuteStep
{
public:
    using Input = model::RequestData;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"newsletters_newsletters_verifications_put_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& ctx = blog::handlers::runtime::ServiceRegistry::context();
        blog::api::MutationStore mutation{ctx.store()};
        
        Output out;
                const auto result = mutation.edit("newsletters", input.id, input.body.empty() ? "{}" : input.body);
        out["status"]=std::to_string(result.status());
        out["body"]=result.body();
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace
