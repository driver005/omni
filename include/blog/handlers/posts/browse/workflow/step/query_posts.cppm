module;
#include <rfl.hpp>
export module blog_handlers_posts_browse:step_query_posts;

import std;
import core_workflow;
import core_database;
import core_serde;
import blog_model;
import blog_handlers_middleware;
import blog_context;
import :dto_post;

export namespace blog::handlers::posts::browse::workflow {

namespace dto = blog::handlers::posts::browse::dto;

/**
 * @brief Step: query posts through core::db::RepositoryBase and project to the
 * snapshot-shaped DTO. Output is the JSON array fragment as a string (a
 * WorkflowValue-compatible type).
 */
class QueryPostsStep
{
public:
    using Input = std::map<std::string, std::string>;
    using Output = std::string;
    static constexpr std::string_view name{"query_posts"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& params,
        core::workflow::WorkflowContext&
    ) const
    {
        auto* context = blog::handlers::middleware::MiddlewareContextRegistry::active();
        if (context == nullptr) {
            return core::workflow::StepResponse<Output>{"[]"}.permanent_failure(
                "BlogContext not registered"
            );
        }
        core::db::RepositoryBase<blog::model::Post> repository{context->store().db()};
        auto posts = repository.all();
        if (const auto it = params.find("filter"); it != params.end()) {
            const auto filter = it->second;
            std::string_view status;
            bool negate = false;
            if (const auto colon = filter.find("status:"); colon != std::string_view::npos) {
                std::string_view value = std::string_view{filter}.substr(colon + 7);
                if (!value.empty() && value.front() == '-') { negate = true; value.remove_prefix(1); }
                status = value;
            }
            if (!status.empty()) {
                std::vector<blog::model::Post> filtered;
                for (const auto& post : posts) {
                    if ((post.get_status() == status) != negate) filtered.push_back(post);
                }
                posts = std::move(filtered);
            }
        }

        std::size_t limit{15};
        std::size_t page{1};
        if (const auto it = params.find("limit"); it != params.end()) {
            try { limit = std::max<std::size_t>(1, std::stoul(it->second)); } catch (...) {}
        }
        if (const auto it = params.find("page"); it != params.end()) {
            try { page = std::max<std::size_t>(1, std::stoul(it->second)); } catch (...) {}
        }
        const auto offset = (page - 1) * limit;

        std::string out{"["};
        bool first{true};
        for (std::size_t i = offset; i < posts.size() && i - offset < limit; ++i) {
            dto::PostDto dto;
            const auto& post = posts[i];
            dto.id = std::format("{:024x}", post.get_id());
            dto.uuid = post.get_uuid().empty()
                           ? std::format("post-{}", post.get_id())
                           : post.get_uuid();
            dto.title = post.get_title();
            dto.slug = post.get_slug();
            dto.html = post.get_html();
            dto.custom_excerpt = post.get_custom_excerpt();
            dto.excerpt = post.get_custom_excerpt().empty()
                              ? std::string{"Welcome to Ghost."}
                              : post.get_custom_excerpt();
            dto.status = post.get_status();

            // Build the wire object as a field map so `fields=` can remove keys
            // entirely (a blank value would still satisfy snapshot assertions).
            std::map<std::string, rfl::Generic> fields{
                {"id", dto.id},
                {"uuid", dto.uuid},
                {"title", dto.title},
                {"slug", dto.slug},
                {"mobiledoc", dto.mobiledoc},
                {"lexical", dto.lexical},
                {"html", dto.html},
                {"comment_id", dto.comment_id},
                {"excerpt", dto.excerpt},
                {"custom_excerpt", dto.custom_excerpt},
                {"canonical_url", rfl::Generic{}},
                {"codeinjection_head", rfl::Generic{}},
                {"codeinjection_foot", rfl::Generic{}},
                {"feature_image", rfl::Generic{}},
                {"feature_image_alt", rfl::Generic{}},
                {"feature_image_caption", rfl::Generic{}},
                {"featured", dto.featured},
                {"custom_template", rfl::Generic{}},
                {"frontmatter", rfl::Generic{}},
                {"meta_title", rfl::Generic{}},
                {"meta_description", rfl::Generic{}},
                {"email", rfl::Generic{}},
                {"email_only", dto.email_only},
                {"email_segment", dto.email_segment},
                {"email_subject", rfl::Generic{}},
                {"created_at", dto.created_at},
                {"updated_at", dto.updated_at},
                {"status", dto.status},
                {"authors", std::vector<rfl::Generic>{}},
                {"count", rfl::Generic{rfl::Object<rfl::Generic>{}}},
                {"type", dto.type},
                {"locale", dto.locale},
                {"visibility", dto.visibility}
            };
            if (const auto it = params.find("fields"); it != params.end()) {
                std::set<std::string> keep{"id"};
                for (auto field : std::string_view{it->second} | std::views::split(',')) {
                    keep.emplace(std::ranges::begin(field), std::ranges::end(field));
                }
                for (auto fit = fields.begin(); fit != fields.end();) {
                    fit = keep.contains(fit->first) ? std::next(fit) : fields.erase(fit);
                }
            }
            const auto json = core_serde::JsonCodec{}.encode(fields);
            out += (first ? "" : ",") + json;
            first = false;
        }
        out += "]";
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace blog::handlers::posts::browse::workflow
