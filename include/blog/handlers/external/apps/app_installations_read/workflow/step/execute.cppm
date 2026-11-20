export module blog_handlers:apps_app_installations_read_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import blog_handlers_middleware;
import :outbox_shim;
import :apps_app_installations_browse_step;

// Strict Ghost parity for `appInstallations.read` (app-installations.ts:65-75):
// options:['id','include'], include∈{'history'}, service.read(id,{withHistory})
// -> repository.find(id) + optional historyOf(id); 404 NotFoundError if absent.
// The `app_installations` envelope mirrors toAppInstallationsResponse.
export namespace blog::handlers::apps::app_installations_read::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"apps_app_installations_read_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        const auto id = path_id(in.get_path());
        if (id.empty()) return result(invalid("id is required", "id"));

        const auto params = query_params(in.get_path());
        const auto include = params.contains("include") ? params.at("include") : std::string{};
        if (!include.empty() && include != "history") {
            return result(invalid("include must be history", "include"));
        }
        return result(read_one(*s_ctx, id, include == "history"));
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    [[nodiscard]] static core::workflow::StepResponse<Output> result(const support::Result& value)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(value.status)}, {"body", value.body}}};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::apps::app_installations_read::workflow
