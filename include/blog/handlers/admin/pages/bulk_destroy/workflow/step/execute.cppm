export module blog_handlers:pages_bulk_destroy_step;
import :domain_all;
import :pages_dto_links;
import std;
import core_workflow;
import core_query;
import core_auth;
import interfaces;

export namespace blog::handlers::pages::bulk_destroy::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"pages_bulk_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::pages::support;
        auto& ctx = *s_ctx;
        auto& mutations = ctx.store();

        const auto principal = principal_for(in);
        core::auth::ghost::GhostAuthorizer authorizer;
        const auto authorized = authorizer.authorize(principal, true, "posts", "destroy");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(authorized.status)}, {"body", authorized.error_json}}
            };
        }

        const auto q = core::query::parse(in.get_path(), std::array<std::string_view, 1>{"filter"});
        if (core::query::has_restricted_field(q.raw_filter)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::ApiError{}.validation(
                      "Restricted field used in bulk filter",
                      "Validation Error",
                      "filter"
                  )}}
            };
        }

        // Ghost #getFilteredBulkPostQuery: status:'all', type:'page', lockForUpdate.
        auto scoped = q;
        scoped.limit = std::numeric_limits<std::size_t>::max();
        // Every row in this store is already a page, so type:page needs no predicate.
        std::string matched;
        try {
            matched = core::query::apply(ctx.store().json_for("pages", {}), scoped, 15);
        } catch (...) {
        }

        const auto ids = ids_from_rows(matched);
        bool all_draft = !ids.empty();
        for (const auto& id: ids) {
            const auto row = ctx.store().find_by_resource("pages", id);
            if (!row || row_field(*row, "status") != "draft") {
                all_draft = false;
                break;
            }
        }

        blog::handlers::outbox::global_outbox().begin_transaction();
        std::size_t successful = 0, unsuccessful = 0;
        for (const auto& id: ids) {
            const auto result = mutations.destroy("pages", id);
            if (result.ok()) {
                ++successful;
            } else {
                ++unsuccessful;
            }
        }
        emit_event(
            "PostsBulkDestroyedEvent",
            "{\"ids\":" + json_ids(ids) + ",\"allDraft\":" + (all_draft ? "true" : "false") + "}",
            ""
        );
        blog::handlers::outbox::global_outbox().commit();

        // pages.bulkDestroy serializer: {bulk:{meta:{stats,errors}}}.
        const auto body = std::string{"{\"bulk\":{\"meta\":{\"stats\":{\"successful\":"} +
                          std::to_string(successful) +
                          ",\"unsuccessful\":" + std::to_string(unsuccessful) +
                          "},\"errors\":[]}}}";
        std::map<std::string, std::string> out{{"status", "200"}, {"body", body}};
        out["x-cache-invalidate"] = all_draft ? "" : "/*";
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::pages::bulk_destroy::workflow
