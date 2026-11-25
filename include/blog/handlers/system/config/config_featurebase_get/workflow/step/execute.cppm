export module blog_handlers:config_config_featurebase_get_step;
import :domain_all;
import :config_browse_step;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::config::config_featurebase_get::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"config_config_featurebase_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        (void)in;
        // Ghost binds this path to api.config.featurebase, which config.js does not
        // export (dead route -> 500). The harness keeps a functional config body here;
        // use the same strict Ghost config computation as GET /config and do NOT emit
        // fabricated pagination or an empty-envelope fixture.
        return core::workflow::StepResponse<Output>{
            blog::handlers::config::browse::workflow::build_config_body(*s_ctx)
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::config::config_featurebase_get::workflow
