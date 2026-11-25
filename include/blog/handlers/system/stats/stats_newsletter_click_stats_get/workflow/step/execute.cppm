export module blog_handlers:stats_stats_newsletter_click_stats_get_step;
import :domain_all;
import :stats_dto_stats_empty_response;
import :stats_dto_stats_member_entry;
import :stats_dto_stats_member_meta;
import :stats_dto_stats_member_response;
import :stats_dto_stats_member_totals;
import :stats_dto_stats_mrr_entry;
import :stats_dto_stats_mrr_meta;
import :stats_dto_stats_mrr_response;
import :stats_dto_stats_mrr_total;
import std;
import core_workflow;
import core_serde;
import core_query;
import interfaces;
import blog_handlers_middleware;

export namespace blog::handlers::stats::stats_newsletter_click_stats_get::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"stats_newsletter_click_stats_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        namespace s = blog::handlers::stats::support;
        static constexpr std::string_view kOptions[]{"newsletter_id", "post_ids", "fail_on_error"};
        const auto q = core::query::parse(in.get_path(), kOptions);
        (void)q;
        const auto nit = q.options.find("newsletter_id");
        if (nit == q.options.end() || nit->second.empty()) {
            return core::workflow::StepResponse<Output>{s::result("200", s::wrap("[]", ""))};
        }
        std::string rows;
        try {
            rows = ctx.store().json_for("stats", s::store_params(q));
        } catch (...) {
            const auto it = q.options.find("fail_on_error");
            if (it != q.options.end() && (it->second == "true" || it->second == "1")) {
                return core::workflow::StepResponse<Output>{
                    s::result("500", s::internal_error("Could not fetch newsletter click stats"))
                };
            }
        }
        const std::string body = s::wrap(rows, "");
        return core::workflow::StepResponse<Output>{s::result("200", body)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::stats::stats_newsletter_click_stats_get::workflow
