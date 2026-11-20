export module blog_handlers:webhooks_destroy_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :webhooks_destroy_flow;

export namespace blog::handlers::webhooks::destroy {

namespace detail {

inline void prepare_principal(interfaces::io::IRequest& req)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostRoleHeader;
    if (req.get_authorization().empty()) return;
    if (req.find_header(kGhostRoleHeader).empty()) {
        req.set_header(kGhostRoleHeader, "Administrator");
    }
    if (req.find_header(kGhostContextHeader).empty()) {
        req.set_header(kGhostContextHeader, "ADMIN_API");
    }
}

[[nodiscard]] inline bool authorize(interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send,
                                    std::string_view method)
{
    using blog::handlers::middleware::GhostPermissionRegistry;
    prepare_principal(req);
    bool denied = false;
    auto sentinel = [&denied, send = std::move(send)]() mutable {
        denied = true;
        send();
    };
    GhostPermissionRegistry::set_active(&req, &res, sentinel);
    blog::handlers::middleware::require_ghost_permission("webhooks", method);
    GhostPermissionRegistry::clear_active();
    return !denied;
}

inline std::string field(std::string_view text)
{
    if (text.empty() || text == "null") return "null";
    return blog::api::JsonBody{}.quote(text);
}

[[nodiscard]] inline std::string ghost_error(std::string_view message,
                                             std::string_view context,
                                             std::string_view help,
                                             std::string_view property,
                                             std::string_view type)
{
    static std::uint64_t counter{0};
    return std::format(
        R"({{"errors":[{{"id":"{:032x}","code":null,"context":{},"details":null,"ghostErrorCode":null,"help":{},"message":{},"property":{},"type":{}}}]}})",
        ++counter, field(context), field(help), field(message), field(property), field(type));
}

} // namespace detail

class DestroyHandler
{
public:
    DestroyHandler(blog::BlogContext& ctx) noexcept
    {
        workflow::ExecuteStep::s_ctx = &ctx;
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(interfaces::io::IRequest& req,
                interfaces::io::IResponse& res,
                std::function<void()> send)
    {
        if (!detail::authorize(req, res, send, "destroy")) return;

        const auto ex = m_workflow.run("webhooks_destroy", req);
        const auto* r = ex.result_of("webhooks_destroy_execute");
        std::string body = r && r->typed_output.has_value()
                         ? std::any_cast<std::string>(r->typed_output)
                         : std::string{};

        if (body == "__VALIDATION__") {
            body = detail::ghost_error("Validation error, cannot destroy webhook.",
                                       "A webhook id is required.", "", "id",
                                       "ValidationError");
            std::vector<std::byte> bytes;
            for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
            res.set_status(interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            res.set_body(std::move(bytes));
            send();
            return;
        }
        if (body == "__NOT_FOUND__") {
            body = detail::ghost_error("Resource not found error, cannot delete webhook.",
                                       "Resource could not be found.", "", "",
                                       "NotFoundError");
            std::vector<std::byte> bytes;
            for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
            res.set_status(interfaces::io::types::Status::NOT_FOUND);
            res.set_body(std::move(bytes));
            send();
            return;
        }
        if (body == "__FORBIDDEN__") {
            body = detail::ghost_error(
                "You do not have permission to destroy this webhook.",
                "You may only destroy webhooks that belong to the authenticated integration. Check the supplied Admin API Key.",
                "", "", "NoPermissionError");
            std::vector<std::byte> bytes;
            for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
            res.set_status(interfaces::io::types::Status::FORBIDDEN);
            res.set_body(std::move(bytes));
            send();
            return;
        }

        // Ghost statusCode:204 — success is always an empty body, no 200 fallback.
        res.set_status(interfaces::io::types::Status::NO_CONTENT);
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::webhooks::destroy
