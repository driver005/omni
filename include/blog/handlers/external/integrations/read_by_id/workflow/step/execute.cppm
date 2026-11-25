export module blog_handlers:integrations_read_by_id_step;
import :domain_all;
import :integrations_dto_more_resources;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::integrations::read_by_id::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"integrations_read_by_id_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        namespace support = blog::handlers::integrations::support;
        auto& ctx = *s_ctx;
        const std::string_view path{in.get_path()};

        const auto params = support::query_options(path, {"include", "id", "require"});
        if (const auto it = params.find("include");
            it != params.end() && !support::valid_include(it->second)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}
                      .validation("Validation error", "Invalid include value", "include")}}
            };
        }

        const std::string id = support::tail_id(path);
        if (id.empty()) {
            return not_found();
        }
        auto record = ctx.store().resources().get("integrations", id);
        if (record.empty()) {
            return not_found();
        }

        // input/integrations.js:18-22 sets the default type filter for read too.
        const std::string type = support::scalar(support::field(record, "type"));
        if (!type.empty() && type != "custom" && type != "builtin" && type != "core") {
            return not_found();
        }

        const std::string include =
            params.contains("include") ? params.at("include") : std::string{};
        record = support::embed_relations(ctx.store(), record, include);
        record = support::prefix_admin_secrets(record);
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", support::wrap("integrations", record)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static core::workflow::StepResponse<Output> not_found()
    {
        return core::workflow::StepResponse<Output>{
            {{"status", "404"},
             {"body", blog::handlers::integrations::support::not_found_integration()}}
        };
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::integrations::read_by_id::workflow
