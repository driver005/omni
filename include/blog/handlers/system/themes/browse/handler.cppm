export module blog_handlers:themes_browse_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :themes_browse_flow;

export namespace blog::handlers::themes::browse {

namespace detail {

    /** @brief Stand-in for Ghost `mw.authAdminApi` (`admin/routes.js:360`). */
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

    /** @brief `permissions:true` under `docName:'themes'`, action `browse` (`themes.js:17`). */
    [[nodiscard]] inline bool authorize(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view method
    )
    {
        prepare_principal(req);
        bool allowed{false};
        auto next = [&allowed](
                        interfaces::io::IRequest&,
                        interfaces::io::IResponse&,
                        std::function<void()>
                    ) noexcept
        {
            allowed = true;
        };
        blog::handlers::middleware::require_ghost_permission(
            req,
            res,
            std::move(next),
            std::move(send),
            "themes",
            method
        );
        return allowed;
    }

} // namespace detail

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
        if (!detail::authorize(req, res, send, "browse")) {
            return;
        }

        const auto ex = m_workflow.run("themes_browse", req);
        const auto* r = ex.result_of("themes_browse_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"body", "{\"themes\":[]}"}};
        const std::string body = payload.contains("body") ? payload.at("body") : std::string{"{}"};

        std::vector<std::byte> bytes;
        for (const char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        // Ghost `headers.cacheInvalidate:false` (`themes.js:14-16`) -> no cache header.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::themes::browse
