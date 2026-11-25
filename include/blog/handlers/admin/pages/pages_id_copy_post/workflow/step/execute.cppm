export module blog_handlers:pages_pages_id_copy_post_step;
import :domain_all;
import :pages_dto_links;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;

export namespace blog::handlers::pages::pages_id_copy_post::workflow {

namespace local_mapping {
    [[nodiscard]] inline std::string
    wrap(std::string_view resource, std::string_view rows, std::string_view meta = {})
    {
        return "{\"" + std::string{resource} + "\":" + std::string{rows} +
               (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
    }

    [[nodiscard]] inline std::string clean(std::string_view json, std::string_view)
    {
        return std::string{json};
    }

    [[nodiscard]] inline std::string pagination(
        std::size_t total,
        std::size_t limit,
        std::size_t page,
        std::size_t pages,
        std::string_view prev,
        std::string_view next
    )
    {
        return std::format(
            R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
            total,
            limit,
            page,
            pages,
            prev.empty() ? std::string{"null"} : std::string{prev},
            next.empty() ? std::string{"null"} : std::string{next}
        );
    }
} // namespace local_mapping

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"pages_pages_id_copy_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::pages::support;
        auto& ctx = *s_ctx;
        auto& mutations = ctx.store();

        const auto principal = principal_for(in);
        core::auth::ghost::GhostAuthorizer authorizer;
        const auto authorized = authorizer.authorize(principal, true, "posts", "add");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(authorized.status)}, {"body", authorized.error_json}}
            };
        }

        const auto q = core::query::parse(in.get_path(), kCopyOptions);
        const std::string raw{in.get_path()};
        auto path = std::string_view{raw}.substr(0, raw.find('?'));
        while (path.size() > 1 && path.back() == '/') {
            path.remove_suffix(1);
        }
        const auto slash = path.find_last_of('/');
        const std::string id{
            slash == std::string_view::npos ? std::string{path}
                                            : std::string{path.substr(slash + 1)}
        };
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::ApiError{}.validation("An id is required.", "Validation Error", "id")}}
            };
        }

        const auto source = ctx.store().find_by_resource("pages", id);
        if (!source) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body", blog::api::ApiError{}.json("Page not found.", "Page not found")}}
            };
        }

        // postsService.copyPost: pick COPIED_POSTS_FIELDS, append " (Copy)", force draft,
        // keep authors/tags/meta/tiers relations (already present on the wire record).
        auto copy = local_mapping::clean(*source, "page");
        const auto title = row_field(copy, "title");
        copy = set_string_field(std::move(copy), "title", title + " (Copy)");
        copy = set_string_field(std::move(copy), "status", "draft");

        blog::handlers::outbox::global_outbox().begin_transaction();
        const auto created = mutations.create("pages", copy);
        if (!created.ok()) {
            blog::handlers::outbox::global_outbox().rollback();
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(created.status())}, {"body", created.body()}}
            };
        }
        emit_event("post.copied", "{\"source_id\":\"" + id + "\"}", id);
        blog::handlers::outbox::global_outbox().commit();

        std::string rows;
        try {
            if (auto value = core_serde::JsonCodec{}.decode<serde::Value>(created.body())) {
                if (auto object = value->to_object()) {
                    if (const auto pages = object->get("pages")) {
                        rows = core_serde::JsonCodec{}.encode(*pages);
                    }
                }
            }
        } catch (...) {
        }
        const auto body = local_mapping::wrap("pages", local_mapping::clean(rows, "page"));
        const auto new_id = row_field(first_row(rows), "id");

        return core::workflow::StepResponse<Output>{
            {{"status", "201"}, {"body", body}, {"location", "/pages/" + new_id + "/"}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::pages::pages_id_copy_post::workflow
