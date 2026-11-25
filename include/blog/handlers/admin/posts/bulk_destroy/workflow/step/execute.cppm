export module blog_handlers:posts_bulk_destroy_step;
import :domain_all;
import :posts_support;

import std;
import core_workflow;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::posts::bulk_destroy::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"posts_bulk_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& input, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::posts::support;
        const auto parsed = parse(input.get_path(), kBulkOptions, true);
        if (!parsed.error.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", parsed.status}, {"body", parsed.error}, {"allDraft", "false"}}
            };
        }

        auto& ctx = *s_ctx;
        const auto ids = parsed.query.raw_filter.empty()
                             ? std::vector<std::string>{}
                             : ids_for_filter(ctx, parsed.query.raw_filter);
        const bool all_draft = all_draft_posts(ctx, ids);

        std::size_t successful = 0;
        std::size_t unsuccessful = 0;
        blog::handlers::outbox::global_outbox().begin_transaction();
        for (const auto& id: ids) {
            const auto result = ctx.store().destroy("posts", id);
            if (result.ok()) {
                ++successful;
                emit("posts.destroyed", R"({"id":")" + id + R"("})", id);
            } else {
                ++unsuccessful;
            }
        }
        emit(
            "posts.bulk_destroyed",
            std::format(
                R"({{"deleteIds":{},"allDraft":{}}})",
                id_list(ids),
                all_draft ? "true" : "false"
            ),
            ""
        );
        blog::handlers::outbox::global_outbox().commit();

        const std::string body = std::format(
            R"({{"successful":{},"unsuccessful":{},"deleteIds":{},"allDraft":{}}})",
            successful,
            unsuccessful,
            id_list(ids),
            all_draft ? "true" : "false"
        );
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", body}, {"allDraft", all_draft ? "true" : "false"}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    [[nodiscard]] static bool
    all_draft_posts(blog::BlogContext& ctx, const std::vector<std::string>& ids)
    {
        if (ids.empty()) {
            return false;
        }
        using namespace blog::handlers::posts::support;
        for (const auto& id: ids) {
            const auto entity = ctx.store().find_by_resource("posts", id);
            if (!entity || field_of(*entity, "status") != "draft") {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] static std::string id_list(const std::vector<std::string>& ids)
    {
        std::string out = "[";
        bool first = true;
        for (const auto& id: ids) {
            if (!first) {
                out += ',';
            }
            out += '"' + id + '"';
            first = false;
        }
        return out + ']';
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::posts::bulk_destroy::workflow
