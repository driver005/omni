export module blog_handlers:featurebase_featurebase_token_get_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import memory_backends;
import :featurebase_featurebase_token_get_flow;

export namespace blog::handlers::featurebase::featurebase_token_get {
class FeaturebaseTokenGetHandler
{
public:
    FeaturebaseTokenGetHandler(blog::BlogContext& ctx) noexcept
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
        // Auth is on the router chain: require_auth (middleware/auth.cppm).
        const auto ex = m_workflow.run("featurebase_featurebase_token_get", req);
        const auto* r = ex.result_of("featurebase_featurebase_token_get_execute");
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
        // Ghost's op sets headers.cacheInvalidate:false, so headers.get emits no
        // X-Cache-Invalidate header (headers.ts:135-144). The admin app installs
        // cacheControl('private') for every route (admin/app.js:24,
        // cache-control.ts:16-19), so only Cache-Control is written here.
        res.set_header(
            interfaces::io::types::Token::CACHE_CONTROL,
            "no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0"
        );
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::featurebase::featurebase_token_get
