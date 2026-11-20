export module blog_handlers:apps_app_installations_destroy_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import blog_handlers_middleware;
import :outbox_shim;
import :apps_app_installations_browse_step;

// Strict Ghost parity for `appInstallations.destroy` (app-installations.ts:105-117):
// statusCode 204, service.uninstall(context,id) -> read(id) + repository.uninstall
// (soft status transition, current_app_id=null, revision+1) + recordAction
// ('uninstalled'). Repeated uninstall is an idempotent 204; no body.
export namespace blog::handlers::apps::app_installations_destroy::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"apps_app_installations_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        const auto id = path_id(in.get_path());
        if (id.empty()) return result(invalid("id is required", "id"));
        return result(uninstall(*s_ctx, blog::handlers::outbox::global_outbox(), id,
                                actor_from_request(in)));
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

} // namespace blog::handlers::apps::app_installations_destroy::workflow
