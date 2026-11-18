export module blog_operations:automations_automations_id_email_preview_post_step;

import std;
import core_workflow;
import blog_handlers_runtime;
import blog_handlers;
import blog_handlers_middleware;
import blog_dto;
import :automations_automations_id_email_preview_post_model;
import :automations_automations_id_email_preview_post_dto;

export namespace blog::handlers::operations::automations::automations_id_email_preview_post::workflow {

class ExecuteStep
{
public:
    using Input = model::RequestData;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"automations_automations_id_email_preview_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& ctx = blog::handlers::runtime::ServiceRegistry::context();
        blog::api::MutationStore mutation{ctx.store()};
        auto& special = blog::handlers::runtime::ServiceRegistry::special<blog::api::SpecialEndpoints>();
        Output out;
                const auto result = special.automation_email_preview("1", input.body);
        out["status"]=std::to_string(result.status());
        out["body"]=result.body();
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace
