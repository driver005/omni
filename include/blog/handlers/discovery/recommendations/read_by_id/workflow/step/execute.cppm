export module blog_handlers:recommendations_read_by_id_step;
import :domain_all;
import :recommendations_browse_step;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;

export namespace blog::handlers::recommendations::read_by_id::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"recommendations_read_by_id_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::recommendations::support;
        auto& ctx = *s_ctx;

        const auto authorized = authorize(principal_for(in), "read");
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

        const auto q = ghost_query(in.get_path(), kReadOptions);
        if (!includes_ok(q)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation_error("Invalid include value", "Validation Error", "include")}}
            };
        }

        const auto row = find_by_id(ctx, id);
        if (!row) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", not_found_error(id)}}
            };
        }

        const bool with_count = include_counts(q);
        const auto serialized = serialize_rows(ctx, {*row}, with_count);
        std::string data;
        try {
            data = core::query::apply(serialized, q, 5);
        } catch (const std::exception&) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  validation_error(
                      "Restricted field used in Ghost query",
                      "Validation Error",
                      "fields"
                  )}}
            };
        }
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", local_mapping::wrap("data", data)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::recommendations::read_by_id::workflow
