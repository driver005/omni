export module blog_handlers:users_read_by_slug_step;
import :domain_all;
import :users_dto_subresources;
import std;
import core_workflow;
import interfaces;
import core_query;

export namespace blog::handlers::users::read_by_slug::workflow {

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
    static constexpr std::string_view name{"users_read_by_slug_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        static constexpr std::string_view kOptions[]{"include", "fields", "debug"};
        const auto q = core::query::parse(in.get_path(), kOptions);
        for (const auto& include: q.include) {
            if (!allowed_include(include)) {
                return core::workflow::StepResponse<Output>{
                    {{"status", "422"},
                     {"body",
                      blog::api::ApiError{}
                          .validation("Validation error", "Invalid include", "include")}}
                };
            }
        }
        const auto slug = last_segment(in.get_path());
        if (slug.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}
                      .validation("Validation error", "A slug is required", "slug")}}
            };
        }
        const auto rows = ctx.store().json_for("users", {{"filter", "slug:" + slug}});
        const auto items = blog::api::parse_array(rows);
        if (items.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body", blog::api::ApiError{}.json("User not found.", "Not Found")}}
            };
        }
        const auto json =
            project(enrich(clean_user(blog::api::record_to_json(items.front())), q), q);
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", local_mapping::wrap("users", json)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static bool allowed_include(std::string_view value)
    {
        return value == "count.posts" || value == "permissions" || value == "roles" ||
               value == "roles.permissions";
    }

    static std::string clean_user(std::string_view json)
    {
        return local_mapping::clean(json, "user-wire");
    }

    static std::string enrich(std::string_view json, const core::query::ParsedQuery& q)
    {
        auto record = blog::api::JsonBody{}.parse_raw(json);
        if (!record.contains("url")) {
            const auto plain = blog::api::JsonBody{}.parse(json);
            const auto it = plain.find("slug");
            record["url"] = blog::api::JsonBody{}.quote(
                "/author/" + (it == plain.end() ? "" : it->second) + "/"
            );
        }
        if (q.include.contains("permissions") && !record.contains("permissions")) {
            record["permissions"] = "[]";
        }
        return blog::api::record_to_json(record);
    }

    static std::string project(std::string_view json, const core::query::ParsedQuery& q)
    {
        if (q.fields.empty()) {
            return std::string{json};
        }
        const auto rows =
            blog::api::parse_array(core::query::apply("[" + std::string{json} + "]", q, 15));
        return rows.empty() ? std::string{"{}"} : blog::api::record_to_json(rows.front());
    }

    static std::string last_segment(std::string_view path)
    {
        path = path.substr(0, path.find('?'));
        while (path.size() > 1 && path.back() == '/') {
            path.remove_suffix(1);
        }
        const auto slash = path.find_last_of('/');
        return std::string{slash == std::string_view::npos ? path : path.substr(slash + 1)};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::users::read_by_slug::workflow
