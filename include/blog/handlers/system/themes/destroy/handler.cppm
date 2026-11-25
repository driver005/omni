export module blog_handlers:themes_destroy_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :themes_destroy_flow;

export namespace blog::handlers::themes::destroy {

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

    [[nodiscard]] inline std::string json_field(std::string_view text)
    {
        if (text.empty() || text == "null") {
            return "null";
        }
        return blog::api::JsonBody{}.quote(text);
    }

    [[nodiscard]] inline std::string
    ghost_error(std::string_view message, std::string_view type, std::string_view property = {})
    {
        static std::uint64_t counter{0};
        return std::format(
            R"({{"errors":[{{"id":"{:032x}","code":null,"context":null,"details":null,"ghostErrorCode":null,"help":null,"message":{},"property":{},"type":{}}}]}})",
            ++counter,
            json_field(message),
            json_field(property),
            json_field(type)
        );
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

    void handle(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!detail::authorize(req, res, send, "destroy")) {
            return;
        }

        const auto ex = m_workflow.run("themes_destroy", req);
        const auto* r = ex.result_of("themes_destroy_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"status", "400"}};
        const std::string status = payload.contains("status") ? payload.at("status") : "204";
        const std::string message =
            payload.contains("message") ? payload.at("message") : std::string{"Invalid theme name"};

        if (status == "400" || status == "404") {
            const std::string error = detail::ghost_error(
                message,
                status == "404" ? "NotFoundError" : "ValidationError",
                "name"
            );
            std::vector<std::byte> bytes;
            for (const char c: error) {
                bytes.push_back(static_cast<std::byte>(c));
            }
            res.set_status(
                status == "404" ? interfaces::io::types::Status::NOT_FOUND
                                : interfaces::io::types::Status::BAD_REQUEST
            );
            res.set_body(std::move(bytes));
            send();
            return;
        }

        // Ghost `statusCode:204` + `headers.cacheInvalidate:true` (`themes.js:160-163`).
        // The matching `site.changed` side effect is emitted through the outbox in the step.
        res.set_header("X-Cache-Invalidate", "/*");
        res.set_status(interfaces::io::types::Status::NO_CONTENT);
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::themes::destroy
