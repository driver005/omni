export module blog_handlers:offers_browse_handler;
import :domain_all;
import :offers_browse_step;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :offers_browse_flow;

export namespace blog::handlers::offers::browse {
class BrowseHandler
{
public:
    BrowseHandler(blog::BlogContext& ctx) noexcept
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
        // Ghost mw.authAdminApi + permissions:true (admin/routes.js:164, offers.js:18).
        if (!support::authorize(req, res, send, "browse")) {
            return;
        }

        const auto ex = m_workflow.run("offers_browse", req);
        const auto* r = ex.result_of("offers_browse_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const auto code = payload.contains("status") ? payload.at("status") : std::string{"200"};
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        auto status = interfaces::io::types::Status::OK;
        if (code == "400") {
            status = interfaces::io::types::Status::BAD_REQUEST;
        } else if (code == "401") {
            status = interfaces::io::types::Status::UNAUTHORIZED;
        } else if (code == "403") {
            status = interfaces::io::types::Status::FORBIDDEN;
        }
        // Ghost headers.cacheInvalidate:false -> no X-Cache-Invalidate header.
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::offers::browse
