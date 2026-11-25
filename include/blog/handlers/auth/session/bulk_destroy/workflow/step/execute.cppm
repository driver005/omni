export module blog_handlers:session_bulk_destroy_step;

import :domain_all;
import :session_create_step;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::session::bulk_destroy::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"session_bulk_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;
        const auto sid = cookie_session_id(in);

        // auth.session.logout -> sessionService.removeUserForSession
        // (session.ts:94, session-service.ts:506-516). Ghost clears fields in
        // place rather than deleting the record; it is idempotent for a missing
        // session.
        auto state = read_session(ctx, sid);
        if (!state.id.empty()) {
            // require_email_mfa is not exposed as a config primitive here, so
            // apply the strictest cleanup (clear verified binding either way).
            state.verified = false;
            state.verified_user_id.clear();
            state.user_id.clear();
            invalidate_challenge(state);
            write_session(ctx, state);
        }
        return core::workflow::StepResponse<Output>{{{"status", "204"}, {"body", ""}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::session::bulk_destroy::workflow
