export module blog_handlers:upgrades_read_by_id_step;
import :domain_all;
import :upgrades_browse_step;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::upgrades::read_by_id::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = support::Output;
    static constexpr std::string_view name{"upgrades_read_by_id_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        // Ghost options:['id'] + parseJobId -> upgradeJobIdSchema regex
        // ^[a-z0-9][a-z0-9-]{0,63}$ (schemas.ts:6). Invalid ids are 422 and do
        // not reach the host adapter.
        const auto id = support::last_segment(in.get_path());
        if (!support::valid_job_id(id)) {
            return core::workflow::StepResponse<Output>{
                support::validation_error("Expected a job id from this host.")
            };
        }
        // Ghost getJob() union: full job | {id,state:'unknown'} | {id,state:'expired'};
        // adapter failures and id mismatches map to 502.
        return core::workflow::StepResponse<Output>{support::read_job(ctx, id)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::upgrades::read_by_id::workflow
