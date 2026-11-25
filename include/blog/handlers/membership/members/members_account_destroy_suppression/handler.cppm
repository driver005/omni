export module blog_handlers:members_account_destroy_suppression_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :members_operations;
import :members_account_destroy_suppression_flow;

export namespace blog::handlers::members::members_account::destroy_suppression {
class MembersAccountDestroySuppressionHandler
{
public:
    MembersAccountDestroySuppressionHandler(
        blog::BlogContext& ctx,
        blog::api::members::MembersOperations& ops
    ) noexcept
    {
        workflow::ExecuteStep::s_ctx = &ctx;
        workflow::ExecuteStep::s_ops = &ops;
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        namespace support = blog::api::members::members_support;
        // Ghost members app: permissions:false; identity from session. No session
        // primitive exists, so x-member-id is the harness stand-in. Anonymous read
        // -> emptyWhenAnonymous 204; anonymous write -> rejectWhenAnonymous 401.
        if (std::string{req.find_header("x-member-id")}.empty()) {
            const std::string b = support::unauthorized();
            std::vector<std::byte> bytes;
            for (char c: b) {
                bytes.push_back(static_cast<std::byte>(c));
            }
            res.set_status(interfaces::io::types::Status::UNAUTHORIZED);
            res.set_body(std::move(bytes));
            send();
            return;
        }
        const auto ex = m_workflow.run("members_account_destroy_suppression", req);
        const auto* r = ex.result_of("members_account_destroy_suppression_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"status", "200"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        // Ghost cacheInvalidate:false -> no X-Cache-Invalidate header.
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::members::members_account::destroy_suppression
