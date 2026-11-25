export module blog_handlers:comments_read_by_id_step;
import :domain_all;
import :comments_model_comment;
import :comments_model_member;
import :comments_model_wire_fixtures;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::comments::read_by_id::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"comments_read_by_id_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        auto params = options(in.get_path(), {"include", "fields", "email", "debug"});
        const auto id = last_segment(in.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}
                      .validation("Validation error", "Missing required option", "id")}}
            };
        }
        params["filter[id]"] = id;
        const auto rows = ctx.store().json_for("comments", params);
        if (rows == "[]") {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", R"({"comments":[]})"}}
            };
        }
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", "{\"comments\":" + rows + "}"}}
        };
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

    static std::string last_segment(std::string_view path)
    {
        auto bare = path.substr(0, path.find('?'));
        while (bare.size() > 1 && bare.back() == '/') {
            bare.remove_suffix(1);
        }
        const auto slash = bare.find_last_of('/');
        return std::string{slash == std::string_view::npos ? bare : bare.substr(slash + 1)};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::comments::read_by_id::workflow
