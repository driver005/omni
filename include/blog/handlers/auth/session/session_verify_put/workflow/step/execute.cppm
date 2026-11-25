export module blog_handlers:session_session_verify_put_step;

import :domain_all;
import :session_create_step;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::session::session_verify_put::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"session_session_verify_put_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        const auto body = blog::api::JsonBody{}.parse(in.body_as_string());
        const auto token = body.contains("token") ? body.at("token") : std::string{};
        const auto sid = cookie_session_id(in);
        if (token.empty() || sid.empty()) {
            return unverified();
        }

        auto& ctx = *s_ctx;
        auto state = read_session(ctx, sid);
        if (!csrf_ok(in, state.origin)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"}, {"body", bad_request("Request made from incorrect origin.")}}
            };
        }

        // verifyAuthCodeForUser -> verifyAuthCode -> totp.verify
        // (session-service.ts:351-359,138-153).  The challenge must exist and
        // be no older than AUTH_CODE_VALIDITY_MS (5 minutes).
        if (!has_valid_challenge(state) || state.user_id.empty()) {
            return unverified();
        }
        const auto secret = admin_session_secret(ctx);
        if (!totp_verify(state.user_id, token, secret, state.auth_code_challenge)) {
            return unverified();
        }

        // verifySession: verified=true, verified_user_id bound, challenge
        // invalidated and the verification-email budget reset.
        verify_session(ctx, state);
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", ""}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    /** Ghost `res.sendStatus(401)` — status only, empty body (middleware.ts:86). */
    [[nodiscard]] static core::workflow::StepResponse<Output> unverified()
    {
        return core::workflow::StepResponse<Output>{{{"status", "401"}, {"body", ""}}};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::session::session_verify_put::workflow
