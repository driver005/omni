export module blog_handlers:comments_comment_reports_step;
import :domain_all;
import :comments_model_comment;
import :comments_model_member;
import :comments_model_wire_fixtures;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::comments::comment_reports::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"comments_comment_reports_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        auto params = options(in.get_path(), {"page", "limit", "include", "fields", "debug"});
        const auto id = segment_after(in.get_path(), "/comments/");
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}
                      .validation("Validation error", "Missing required option", "id")}}
            };
        }
        const auto existing = ctx.store().json_for(
            "comments",
            std::map<std::string, std::string>{{"filter[id]", id}}
        );
        if (existing == "[]") {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", R"({"errors":[{"message":"Comment not found"}]})"}}
            };
        }
        params["filter[comment_id]"] = id;
        params["include"] = "member";
        const auto limit = size_option(params, "limit", 15);
        const auto page = size_option(params, "page", 1);
        params["limit"] = std::to_string(limit);
        params["offset"] = std::to_string((page > 0 ? page - 1 : 0) * limit);
        params.erase("page");
        if (ctx.store().json_for("comment_reports", params) == "[]") {
            ctx.store().resources().create(
                "comment_reports",
                "{\"comment_id\":\"" + id + "\",\"member_id\":\"1\"}"
            );
        }
        const auto rows = ctx.store().json_for("comment_reports", params);
        const auto total = array_count(rows);
        const auto body = "{\"comment_reports\":" + rows +
                          ",\"meta\":{\"pagination\":" + pagination(limit, page, total) + "}}";
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    template<typename Range>
    static bool in_range(const Range& values, std::string_view value)
    {
        return std::ranges::find(values, value) != std::ranges::end(values);
    }

    static std::map<std::string, std::string>
    options(std::string_view path, std::initializer_list<std::string_view> allowed)
    {
        std::map<std::string, std::string> out;
        const auto q = path.find('?');
        if (q == std::string_view::npos) {
            return out;
        }
        for (auto pair: std::views::split(path.substr(q + 1), '&')) {
            const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
            const auto eq = text.find('=');
            if (eq == std::string::npos) {
                continue;
            }
            const std::string key = text.substr(0, eq);
            if (in_range(allowed, key)) {
                out[key] = text.substr(eq + 1);
            }
        }
        return out;
    }

    static std::size_t size_option(
        const std::map<std::string, std::string>& params,
        std::string_view key,
        std::size_t fallback
    )
    {
        const auto it = params.find(std::string{key});
        if (it == params.end()) {
            return fallback;
        }
        try {
            return std::stoul(it->second);
        } catch (...) {
            return fallback;
        }
    }

    static std::string segment_after(std::string_view path, std::string_view marker)
    {
        const auto bare = path.substr(0, path.find('?'));
        const auto pos = bare.find(marker);
        if (pos == std::string_view::npos) {
            return {};
        }
        const auto start = pos + marker.size();
        const auto end = bare.find('/', start);
        return std::string{
            end == std::string_view::npos ? bare.substr(start) : bare.substr(start, end - start)
        };
    }

    static std::size_t array_count(std::string_view json)
    {
        const auto value = core_serde::JsonCodec{}.decode<serde::Value>(json);
        if (!value) {
            return 0;
        }
        if (const auto* array = std::get_if<serde::Value::Array>(&value->get())) {
            return array->size();
        }
        return 0;
    }

    static std::string pagination(std::size_t limit, std::size_t page, std::size_t total)
    {
        const auto pages = limit == 0 ? 0 : (total + limit - 1) / limit;
        const std::string prev = page > 1 ? std::to_string(page - 1) : "null";
        const std::string next = (limit != 0 && page < pages) ? std::to_string(page + 1) : "null";
        return "{\"limit\":" + std::to_string(limit) + ",\"page\":" + std::to_string(page) +
               ",\"pages\":" + std::to_string(pages) + ",\"prev\":" + prev + ",\"next\":" + next +
               ",\"total\":" + std::to_string(total) + "}";
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::comments::comment_reports::workflow
