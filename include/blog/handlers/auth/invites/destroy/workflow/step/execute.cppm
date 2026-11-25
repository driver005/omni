export module blog_handlers:invites_destroy_step;
import :domain_all;
import :invites_dto_admin_resources;
import std;
import core_workflow;
import core_query;
import interfaces;

export namespace blog::handlers::invites::destroy::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"invites_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        // Ghost options:['include','id'], include∈[] (invites.js:63-68).
        static constexpr std::string_view kOptions[]{"include", "id"};
        const auto q = core::query::parse(in.get_path(), kOptions);
        if (!q.include.empty()) {
            return invalid("include", "include is not supported");
        }

        const std::string id = last_segment(in.get_path());
        if (id.empty()) {
            return invalid("id", "An id is required");
        }

        // models.Invite.destroy({...frame.options, require:true}) (invites.js:70-72).
        const auto result = ctx.store().destroy("invites", id);
        if (!result.ok() || result.status() == 404) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body", blog::api::ApiError{}.json("Invite not found.", "Not Found")}}
            };
        }

        // actionsResourceType:'invite' / actionsExtraContext (model/invite.js:22-24).
        support::emit(
            "invite.destroyed",
            "{\"id\":" + support::quote(id) + ",\"status\":null,\"role_id\":null}",
            id
        );

        // statusCode:204 -> empty body (invites.js:62; default.js:15-20).
        return core::workflow::StepResponse<Output>{{{"status", "204"}, {"body", ""}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static std::string last_segment(std::string_view path)
    {
        auto clean_path = path.substr(0, path.find('?'));
        while (clean_path.size() > 1 && clean_path.back() == '/') {
            clean_path.remove_suffix(1);
        }
        const auto slash = clean_path.find_last_of('/');
        return std::string{
            slash == std::string_view::npos ? clean_path : clean_path.substr(slash + 1)
        };
    }

    static core::workflow::StepResponse<Output>
    invalid(std::string_view property, std::string_view context)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", "422"},
             {"body", blog::api::ApiError{}.validation("Validation error", context, property)}}
        };
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::invites::destroy::workflow
