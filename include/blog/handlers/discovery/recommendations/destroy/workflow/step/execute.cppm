export module blog_handlers:recommendations_destroy_step;
import :domain_all;
import :recommendations_browse_step;
import std;
import core_workflow;
import core_auth;
import interfaces;

export namespace blog::handlers::recommendations::destroy::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"recommendations_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::recommendations::support;
        auto& ctx = *s_ctx;

        const auto authorized = authorize(principal_for(in), "destroy");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(authorized.status)}, {"body", authorized.error_json}}
            };
        }

        const std::string id = path_id(in.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation_error("id is required", "Validation Error", "id")}}
            };
        }

        const auto existing = find_by_id(ctx, id);
        if (!existing) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", not_found_error(id)}}
            };
        }
        const auto url = field_string(as_object(&*existing), "url");
        if (!ctx.store().resources().destroy("recommendations", id)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", not_found_error(id)}}
            };
        }

        // destroy -> recommendations_enabled setting + webmention delete.
        emit_crud_side_effects("deleted", id, url, !all_rows(ctx).empty());
        return core::workflow::StepResponse<Output>{{{"status", "204"}, {"body", ""}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::recommendations::destroy::workflow
