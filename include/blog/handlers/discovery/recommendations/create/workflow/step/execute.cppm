export module blog_handlers:recommendations_create_step;
import :domain_all;
import :recommendations_browse_step;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;

export namespace blog::handlers::recommendations::create::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"recommendations_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::recommendations::support;
        auto& ctx = *s_ctx;

        const auto authorized = authorize(principal_for(in), "add");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(authorized.status)}, {"body", authorized.error_json}}
            };
        }

        const auto q = ghost_query(in.get_path(), kWriteOptions);
        if (!includes_ok(q)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation_error("Invalid include value", "Validation Error", "include")}}
            };
        }

        const auto inner = unwrap_recommendation(in.body_as_string());
        if (!inner) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  validation_error(
                      "Invalid recommendation payload",
                      "Validation Error",
                      "recommendations"
                  )}}
            };
        }
        const auto* o = as_object(&*inner);
        const auto title = field_string(o, "title");
        const auto url = field_string(o, "url");
        if (title.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation_error("title is required", "Validation Error", "title")}}
            };
        }
        if (url.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation_error("url is required", "Validation Error", "url")}}
            };
        }

        // getByUrl: host/pathname normalized duplicate check.
        if (find_by_url(ctx, url)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  validation_error(
                      "A recommendation with this URL already exists.",
                      "Validation (Unique) failed for url",
                      "url"
                  )}}
            };
        }

        const auto id =
            ctx.store().resources().create("recommendations", materialize_input(*inner));
        const auto stored = ctx.store().resources().get("recommendations", id);
        const auto row = decode(stored);
        if (!row) {
            return core::workflow::StepResponse<Output>{
                {{"status", "201"}, {"body", local_mapping::wrap("data", "[]")}}
            };
        }

        const auto serialized = serialize_rows(ctx, {*row}, include_counts(q));
        std::string data;
        try {
            data = core::query::apply(serialized, q, 5);
        } catch (const std::exception&) {
            data = serialized;
        }
        const auto body = local_mapping::wrap("data", data);

        // add -> recommendations_enabled setting + fire-and-forget webmention.
        emit_crud_side_effects("added", id, url, true);
        return core::workflow::StepResponse<Output>{{{"status", "201"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::recommendations::create::workflow
