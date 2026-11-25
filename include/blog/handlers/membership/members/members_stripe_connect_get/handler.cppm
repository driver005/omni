export module blog_handlers:members_members_stripe_connect_get_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :members_operations;
import :members_members_stripe_connect_get_flow;

export namespace blog::handlers::members::members_stripe_connect_get {
class MembersStripeConnectGetHandler
{
public:
    MembersStripeConnectGetHandler(blog::BlogContext& ctx) noexcept
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
        namespace support = blog::api::members::members_support;
        // Ghost mw.authAdminApi + permissions:true -> members_stripe_connect:auth.
        if (!support::enforce(req, res, send, "members_stripe_connect", "auth")) {
            return;
        }
        const auto ex = m_workflow.run("members_members_stripe_connect_get", req);
        const auto* r = ex.result_of("members_members_stripe_connect_get_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"status", "422"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 422;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{};
        // Ghost response-function path: no X-Cache-Invalidate header; admin app
        // adds Cache-Control: private.
        res.set_header("Cache-Control", "private");
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        if (status == 302 && payload.contains("location")) {
            res.set_header("Location", payload.at("location"));
            res.set_status(interfaces::io::types::Status::FOUND);
        } else {
            res.set_status(static_cast<interfaces::io::types::Status>(status));
        }
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::members::members_stripe_connect_get
