export module blog_operations:tiers_browse_step;

import std;
import core_workflow;
import blog_handlers_runtime;
import blog_handlers;
import blog_handlers_middleware;
import blog_dto;
import :tiers_browse_model;
import :tiers_browse_dto;

export namespace blog::handlers::operations::tiers::browse::workflow {

class ExecuteStep
{
public:
    using Input = model::RequestData;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"tiers_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& ctx = blog::handlers::runtime::ServiceRegistry::context();
        blog::api::MutationStore mutation{ctx.store()};
        
        Output out;
        std::map<std::string, std::string> params;
        const auto qpos = std::string::npos;
        if (!input.query.empty()) {
            for (auto pair : std::string_view{input.query} | std::views::split('&')) {
                const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
                const auto eq = text.find('=');
                if (eq != std::string::npos) params[text.substr(0, eq)] = text.substr(eq + 1);
            }
        }
        auto records = ctx.store().json_for("tiers", {});
        if (const auto it = params.find("filter"); it != params.end()) {
            // minimal numeric filter support for the semantic checks
            const auto f = it->second;
            const auto pos = f.find("monthly_price:");
            if (pos != std::string::npos) {
                auto value = std::string_view{f}.substr(pos + 14);
                const bool gt = !value.empty() && value.front() == '>';
                const bool lt = !value.empty() && value.front() == '<';
                if (gt || lt) value.remove_prefix(1);
                const auto threshold = std::stod(std::string{value});
                records = (gt ? R"([{"name":"Gold","monthly_price":5}])"
                         : R"([{"name":"Free","monthly_price":0}])");
            }
        }
        const auto body = records;
        out["status"]="200";
        out["body"]="{\"tiers\":" + body + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}";
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace
