export module blog_handlers:schedules_put_detail_handler;
import :domain_all;
import :posts_support;
import :outbox_shim;
import std;
import interfaces;
import core_workflow;
import memory_backends;
import :schedules_put_detail_step;
import :schedules_put_detail_flow;

export namespace blog::handlers::schedules::put_detail {

class PutDetailHandler {
public:
    explicit PutDetailHandler(blog::BlogContext& ctx) noexcept
    {
        workflow::ExecuteStep::s_ctx = &ctx;
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                std::function<void()> send)
    {
        using blog::handlers::posts::support::authorize;

        // Ghost pipeline order: validation -> input serialization -> permissions -> query.
        const auto pf = workflow::ExecuteStep::preflight(req.get_path());
        if (!pf.ok) {
            write(res, pf.status, pf.body, {}, std::move(send));
            return;
        }

        // authAdminApiWithUrl: the scheduler may supply the admin token in the URL.
        apply_url_token(req);
        // permissions.canThis(context).publish.post(id), NotFound swallowed by NO_OP
        // in the step. "posts"/"pages" are the plural doc names used by the shared
        // Ghost authorizer (core_auth:ghost).
        if (!authorize(req, res, send, pf.resource, "publish")) return;

        const auto ex = m_workflow.run("schedules_put_detail", req);
        const auto* r = ex.result_of("schedules_put_detail_execute");
        const auto out = r && r->typed_output.has_value()
            ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
            : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const auto status = out.contains("status") ? out.at("status") : std::string{"200"};
        const auto body = out.contains("body") ? out.at("body") : std::string{"{}"};
        const auto invalidate = out.contains("invalidate") ? out.at("invalidate") : std::string{};
        write(res, status, body, invalidate, std::move(send));
    }

private:
    static void apply_url_token(interfaces::io::IRequest& req)
    {
        if (!req.get_authorization().empty()) return;
        const std::string path{req.get_path()};
        const auto q = path.find('?');
        if (q == std::string::npos) return;
        const std::string_view path_view{path};
        const std::string_view query = path_view.substr(q + 1);
        for (auto part : query | std::views::split('&')) {
            const std::string_view kv{std::ranges::begin(part), std::ranges::end(part)};
            if (!kv.starts_with("token=")) continue;
            const auto token = kv.substr(6);
            if (!token.empty()) req.set_header("Authorization", std::string{token});
            return;
        }
    }

    static void write(interfaces::io::IResponse& res, std::string_view status,
                      std::string_view body, std::string_view invalidate,
                      std::function<void()> send)
    {
        std::vector<std::byte> bytes;
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
        if (!invalidate.empty()) res.set_header("X-Cache-Invalidate", invalidate);
        res.set_status(static_cast<interfaces::io::types::Status>(std::stoi(std::string{status})));
        res.set_body(std::move(bytes));
        send();
    }

    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::schedules::put_detail
