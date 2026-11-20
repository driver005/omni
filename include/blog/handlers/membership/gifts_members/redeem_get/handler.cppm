export module blog_handlers:gifts_members_redeem_get_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :gifts_members_redeem_get_flow;

export namespace blog::handlers::gifts_members::redeem_get {

class RedeemGetHandler {
public:
    RedeemGetHandler(blog::BlogContext& ctx) noexcept
    {
        workflow::ExecuteStep::s_ctx = &ctx;
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send)
    {
        const auto ex = m_workflow.run("gifts_members_redeem_get", req);
        const auto* r = ex.result_of("gifts_members_redeem_get_execute");
        const auto payload = r && r->typed_output.has_value()
            ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
            : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (const char c : body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        // Ghost headers.cacheInvalidate:false emits NO invalidation header (headers.ts:135).
        // The members app sets cacheControl('private') at app level (members/app.js:27);
        // mirror that value here because the C++ router has no app-level equivalent.
        res.set_header("Cache-Control",
            "no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0");
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::gifts_members::redeem_get
