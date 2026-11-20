export module blog_handlers:session_session_verify_post_step;

import :domain_all;
import :session_create_step;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::session::session_verify_post::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"session_session_verify_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;
        const auto sid = cookie_session_id(in);
        auto state = read_session(ctx, sid);

        // sendAuthCodeToUser: cookieCsrfProtection + user_id guard
        // (session-service.ts:424-432).
        if (!csrf_ok(in, state.origin)) {
            return core::workflow::StepResponse<Output>{{{"status", "400"},
                {"body", bad_request("Request made from incorrect origin.")}}};
        }
        if (state.user_id.empty()) {
            return core::workflow::StepResponse<Output>{{{"status", "400"},
                {"body", bad_request("Could not fetch user from the session.")}}};
        }

        // rotateAuthCodeChallenge + generateAuthCodeForUser + GhostMailer.send
        // (session-service.ts:439-483).  Rate limiting and IP geolocation have
        // no registered primitive; the mail is emitted through the outbox shim.
        (void)send_auth_code(ctx, in, state);
        write_session(ctx, state);
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", ""}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::session::session_verify_post::workflow
