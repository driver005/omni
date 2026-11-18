export module blog_operations:stats_stats_newsletter_basic_stats_get_step;

import std;
import core_workflow;
import blog_handlers_runtime;
import blog_handlers;
import blog_handlers_middleware;
import blog_dto;
import :stats_stats_newsletter_basic_stats_get_model;
import :stats_stats_newsletter_basic_stats_get_dto;

export namespace blog::handlers::operations::stats::stats_newsletter_basic_stats_get::workflow {

class ExecuteStep
{
public:
    using Input = model::RequestData;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"stats_stats_newsletter_basic_stats_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& ctx = blog::handlers::runtime::ServiceRegistry::context();
        blog::api::MutationStore mutation{ctx.store()};
        auto& special = blog::handlers::runtime::ServiceRegistry::special<blog::api::SpecialEndpoints>();
        Output out;
                const auto result = special.stats_empty();
        out["status"]=std::to_string(result.status());
        out["body"]=result.body();
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace
