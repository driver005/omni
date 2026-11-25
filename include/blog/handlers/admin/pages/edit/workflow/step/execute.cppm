export module blog_handlers:pages_edit_step;
import :domain_all;
import :pages_dto_links;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;

export namespace blog::handlers::pages::edit::workflow {

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
    static constexpr std::string_view name{"pages_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::pages::support;
        auto& ctx = *s_ctx;
        auto& mutations = ctx.store();

        const auto principal = principal_for(in);
        core::auth::ghost::GhostAuthorizer authorizer;
        const auto authorized = authorizer.authorize(principal, true, "posts", "edit");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(authorized.status)}, {"body", authorized.error_json}}
            };
        }

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

        const auto q = core::query::parse(in.get_path(), kEditOptions);
        if (q.options.contains("id") && q.options.at("id") != id) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::ApiError{}
                      .validation("id does not match the path", "Validation Error", "id")}}
            };
        }
        const auto source = q.options.contains("source") ? q.options.at("source") : "";
        if (id.empty() || (!source.empty() && source != "html") ||
            !tokens_ok(
                q.options.contains("include") ? q.options.at("include") : "",
                kAdminIncludes
            ) ||
            !tokens_ok(q.options.contains("formats") ? q.options.at("formats") : "", kFormats)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::ApiError{}
                      .validation("Invalid id/source/include", "Validation Error", "id")}}
            };
        }

        std::string payload = in.body_as_string().empty() ? "{}" : in.body_as_string();
        try {
            if (auto value = core_serde::JsonCodec{}.decode<serde::Value>(payload)) {
                const auto pages = field_raw(*value, "pages");
                if (!pages.empty() && pages != "null") {
                    const auto first = first_row(pages);
                    if (!first.empty() && first != "null") {
                        payload = first;
                    }
                }
            }
        } catch (...) {
        }

        for (const char* attr: {"status", "authors", "visibility"}) {
            if (payload.find(std::string{"\""} + attr + "\"") != std::string::npos) {
                const auto unsafe = authorizer.unsafe_attrs(principal, attr, "");
                if (!unsafe.allowed) {
                    return core::workflow::StepResponse<Output>{
                        {{"status", std::to_string(unsafe.status)}, {"body", unsafe.error_json}}
                    };
                }
            }
        }

        // Pre-edit state for postsService.handleCacheInvalidation.
        std::string previousStatus, uuid;
        if (const auto existing = ctx.store().find_by_resource("pages", id)) {
            previousStatus = row_field(*existing, "status");
            uuid = row_field(*existing, "uuid");
        }
        const auto newStatus = row_field(payload, "status");
        const bool changed = newStatus != previousStatus;

        const auto result =
            mutations.edit("pages", id, set_string_field(std::move(payload), "type", "page"));
        if (!result.ok()) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(result.status())}, {"body", result.body()}}
            };
        }

        std::string rows;
        try {
            if (auto value = core_serde::JsonCodec{}.decode<serde::Value>(result.body())) {
                if (auto object = value->to_object()) {
                    if (const auto pages = object->get("pages")) {
                        rows = core_serde::JsonCodec{}.encode(*pages);
                    }
                }
            }
        } catch (...) {
        }
        const auto body = local_mapping::wrap("pages", local_mapping::clean(rows, "page"));

        std::map<std::string, std::string> out{
            {"status", std::to_string(result.status())},
            {"body", body}
        };
        // Ghost handleCacheInvalidation: publish (or unpublish from published) -> "/*".
        if ((newStatus == "published" && changed) ||
            (newStatus == "draft" && previousStatus == "published")) {
            out["x-cache-invalidate"] = "/*";
        } else if (newStatus == "draft" || (newStatus == "scheduled" && changed)) {
            // Ghost returns preview URLs for draft/scheduled transitions.
            std::string previews;
            for (const char* member: {"free", "members", "paid"}) {
                if (!previews.empty()) {
                    previews += ',';
                }
                previews += "/p/" + uuid + "/?member_status=" + member;
            }
            out["x-cache-invalidate"] = previews;
        }
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::pages::edit::workflow
