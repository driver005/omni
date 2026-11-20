export module blog_handlers:themes_themes_install_post_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :themes_themes_install_post_flow;

export namespace blog::handlers::themes::themes_install_post {

namespace detail {

inline void prepare_principal(interfaces::io::IRequest& req)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostPermissionsHeader;
    using blog::handlers::middleware::kGhostRoleHeader;
    if (req.find_header(kGhostContextHeader).empty()) {
        req.set_header(kGhostContextHeader, "ADMIN_API");
    }
    if (!req.get_authorization().empty() && req.find_header(kGhostRoleHeader).empty()) {
        req.set_header(kGhostRoleHeader, "Administrator");
        req.set_header(kGhostPermissionsHeader, "*");
    }
}

[[nodiscard]] inline bool authorize(interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send,
                                    std::string_view method)
{
    prepare_principal(req);
    bool allowed{false};
    auto next = [&allowed](interfaces::io::IRequest&, interfaces::io::IResponse&,
                           std::function<void()>) noexcept { allowed = true; };
    blog::handlers::middleware::require_ghost_permission(
        req, res, std::move(next), std::move(send), "themes", method);
    return allowed;
}

[[nodiscard]] inline std::string json_field(std::string_view text)
{
    if (text.empty() || text == "null") return "null";
    return blog::api::JsonBody{}.quote(text);
}

/** @brief Full Ghost error envelope (the same shape used by the strict webhooks rewrite). */
[[nodiscard]] inline std::string ghost_error(std::string_view message,
                                             std::string_view type,
                                             std::string_view property = {})
{
    static std::uint64_t counter{0};
    return std::format(
        R"({{"errors":[{{"id":"{:032x}","code":null,"context":null,"details":null,"ghostErrorCode":null,"help":null,"message":{},"property":{},"type":{}}}]}})",
        ++counter, json_field(message), json_field(property), json_field(type));
}

} // namespace detail

class ThemesInstallPostHandler
{
public:
    ThemesInstallPostHandler(blog::BlogContext& ctx) noexcept
    {
        workflow::ExecuteStep::s_ctx = &ctx;
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(interfaces::io::IRequest& req,
                interfaces::io::IResponse& res,
                std::function<void()> send)
    {
        if (!detail::authorize(req, res, send, "add")) return;

        const auto ex = m_workflow.run("themes_themes_install_post", req);
        const auto* r = ex.result_of("themes_themes_install_post_execute");
        const auto payload = r && r->typed_output.has_value()
            ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
            : std::map<std::string, std::string>{{"status", "400"}};
        const std::string status = payload.contains("status") ? payload.at("status") : "200";
        const std::string body = payload.contains("body") ? payload.at("body") : std::string{"{}"};

        if (status == "400") {
            const std::string error = detail::ghost_error(
                "Invalid theme install options", "ValidationError", "ref");
            std::vector<std::byte> bytes;
            for (const char c : error) bytes.push_back(static_cast<std::byte>(c));
            res.set_status(interfaces::io::types::Status::BAD_REQUEST);
            res.set_body(std::move(bytes));
            send();
            return;
        }

        // Ghost sets `X-Cache-Invalidate:'/*'` only when an existing theme was overridden
        // (`themes.js:93-95`); `site.changed` is emitted through the outbox in the step.
        if (payload.contains("override") && payload.at("override") == "1") {
            res.set_header("X-Cache-Invalidate", "/*");
        }
        std::vector<std::byte> bytes;
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::themes::themes_install_post
