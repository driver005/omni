export module blog_handlers:comments_undislike_step;
import :domain_all;
import :comments_model_comment;
import :comments_model_member;
import :comments_model_wire_fixtures;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::comments::undislike::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"comments_undislike_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const auto id = last_segment(in.get_path());
        if (id.empty()) {
            return invalid("id");
        }
        const auto rows = ctx.store().json_for(
            "comment_likes",
            std::map<std::string, std::string>{{"filter[comment_id]", id}, {"filter[score]", "-1"}}
        );
        for (const auto& like_id: record_ids(rows)) {
            ctx.store().resources().destroy("comment_likes", like_id);
        }
        return core::workflow::StepResponse<Output>{{{"status", "204"}, {"body", ""}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static std::string last_segment(std::string_view path)
    {
        auto bare = path.substr(0, path.find('?'));
        while (bare.size() > 1 && bare.back() == '/') {
            bare.remove_suffix(1);
        }
        const auto slash = bare.find_last_of('/');
        return std::string{slash == std::string_view::npos ? bare : bare.substr(slash + 1)};
    }

    static std::vector<std::string> record_ids(std::string_view json)
    {
        std::vector<std::string> out;
        std::size_t from = 0;
        while (true) {
            const auto pos = json.find("\"id\":", from);
            if (pos == std::string_view::npos) {
                break;
            }
            const auto begin = json.find('"', pos + 5);
            if (begin == std::string_view::npos) {
                break;
            }
            const auto end = json.find('"', begin + 1);
            if (end == std::string_view::npos) {
                break;
            }
            out.push_back(std::string{json.substr(begin + 1, end - begin - 1)});
            from = end + 1;
        }
        return out;
    }

    static core::workflow::StepResponse<Output> invalid(std::string_view property)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", "422"},
             {"body",
              blog::api::ApiError{}
                  .validation("Validation error", "Missing required option", property)}}
        };
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::comments::undislike::workflow
