export module blog_handlers:stats_stats_posts_visitor_counts_post_step;
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

export namespace blog::handlers::stats::stats_posts_visitor_counts_post::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"stats_posts_visitor_counts_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        namespace s = blog::handlers::stats::support;
        static constexpr std::span<const std::string_view> kOptions{};
        const auto q = core::query::parse(in.get_path(), kOptions);
        (void)q;
        const std::string body =
            in.body_as_string().empty() ? std::string{"{}"} : std::string{in.body_as_string()};
        const auto ids = s::body_array(body, "postUuids");
        if (!ids || ids->empty()) {
            return core::workflow::StepResponse<Output>{
                s::result("422", s::invalid("postUuids required array", "postUuids", "body"))
            };
        }
        const auto joined = s::join(*ids, ",");
        s::record_tinybird("api_post_visitor_counts", "post_uuids=" + joined);
        const auto rows = ctx.store().json_for("posts", {{"filter[uuid]", joined}});
        const std::size_t count = blog::api::parse_array(rows).size();
        std::string counts;
        for (const auto& id: *ids) {
            if (!counts.empty()) {
                counts += ',';
            }
            counts += s::quote(id) + ":" + std::to_string(count);
        }
        return core::workflow::StepResponse<Output>{s::result("200", s::wrap("[" + counts + "]"))};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::stats::stats_posts_visitor_counts_post::workflow
