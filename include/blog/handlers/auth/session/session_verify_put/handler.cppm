export module blog_handlers:session_session_verify_put_handler;

import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :session_session_verify_put_flow;

export namespace blog::handlers::session::session_verify_put {

class SessionVerifyPutHandler
{
public:
    SessionVerifyPutHandler(blog::BlogContext& ctx) noexcept
    {
        workflow::ExecuteStep::s_ctx = &ctx;
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Ghost has no admin auth here; only rateLimiting.userVerification
        // (routes.js:430-434), which has no registered primitive.
        const auto ex = m_workflow.run("session_session_verify_put", req);
        const auto* r = ex.result_of("session_session_verify_put_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"status", "401"}, {"body", ""}};
        const int code = payload.contains("status") ? std::stoi(payload.at("status")) : 401;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{};

        // 200 verified / 401 unverified / 400 CSRF: status + body straight from
        // the step (401 is sendStatus -> empty body, 400 is a BadRequestError
        // envelope).  No {data,meta} envelope.
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        res.set_status(static_cast<interfaces::io::types::Status>(code));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::session::session_verify_put
