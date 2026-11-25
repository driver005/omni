export module blog_handlers:settings_settings_access_code_regenerate_post_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :settings_browse_step;
import :settings_settings_access_code_regenerate_post_flow;

export namespace blog::handlers::settings::settings_access_code_regenerate_post {
class SettingsAccessCodeRegeneratePostHandler
{
public:
    SettingsAccessCodeRegeneratePostHandler(blog::BlogContext& ctx) noexcept
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
        if (!support::authorize(req, res, send, support::kDocName, "edit")) {
            return;
        }

        const auto ex = m_workflow.run("settings_settings_access_code_regenerate_post", req);
        const auto* r = ex.result_of("settings_settings_access_code_regenerate_post_execute");
        auto payload = r && r->typed_output.has_value()
                           ? std::any_cast<support::Result>(r->typed_output)
                           : support::Result{{"status", "200"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        if (payload.contains("cache_invalidate")) {
            res.set_header("X-Cache-Invalidate", payload.at("cache_invalidate"));
        }
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::settings::settings_access_code_regenerate_post
