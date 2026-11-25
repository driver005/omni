export module blog_handlers:pages_gift_links_post_handler;
import :domain_all;
import :gift_links_ops;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :pages_gift_links_post_flow;

export namespace blog::handlers::pages::gift_links_post {
class GiftLinksPostHandler
{
public:
    GiftLinksPostHandler(blog::BlogContext& ctx) noexcept
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
        // Ghost: mw.authAdminApi + assertCanEditAndGift(manage.gift_link, edit.post).
        if (!blog::api::gift_links_ops::permitted(req, res, send, "gift_links", "manage")) {
            return;
        }
        if (!blog::api::gift_links_ops::permitted(req, res, send, "pages", "edit")) {
            return;
        }
        const auto ex = m_workflow.run("pages_gift_links_post", req);
        const auto* r = ex.result_of("pages_gift_links_post_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::pages::gift_links_post
