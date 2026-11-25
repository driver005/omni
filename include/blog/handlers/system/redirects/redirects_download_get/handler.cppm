export module blog_handlers:redirects_redirects_download_get_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :redirects_redirects_download_get_flow;

export namespace blog::handlers::redirects::redirects_download_get {

namespace download_handler_detail {

    // Ghost headers.yaml sets `Content-Length: Buffer.byteLength(JSON.stringify(result))`
    // (headers.ts:72-78) even though the body is sent via res.send(result) as plain
    // text. Mirror that exact value; JSON.stringify of a string quotes/escapes it.
    [[nodiscard]] inline std::size_t json_string_byte_length(std::string_view text) noexcept
    {
        std::size_t bytes = 2; // opening and closing double quotes
        for (const unsigned char c: text) {
            switch (c) {
                case '"':
                case '\\':
                case '\b':
                case '\f':
                case '\n':
                case '\r':
                case '\t':
                    bytes += 2;
                    break;
                default:
                    bytes +=
                        c < 0x20 ? 6 : 1; // \u00XX for other control chars, UTF-8 bytes otherwise
                    break;
            }
        }
        return bytes;
    }

} // namespace download_handler_detail

class RedirectsDownloadGetHandler
{
public:
    RedirectsDownloadGetHandler(blog::BlogContext& ctx) noexcept
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
        using blog::handlers::middleware::kGhostContextHeader;
        using blog::handlers::middleware::kGhostPermissionsHeader;
        using blog::handlers::middleware::kGhostRoleHeader;

        // Ghost route middleware is mw.authAdminApi + permissions:true with
        // docName:'redirects' and the custom action `download` (redirects.js:12,22,
        // routes.js:505). Route the check through the shared Ghost permission
        // service: no identity -> Ghost 401 envelope, known-but-unprivileged -> 403.
        if (req.find_header(kGhostContextHeader).empty()) {
            req.set_header(kGhostContextHeader, "ADMIN_API");
        }
        if (!req.get_authorization().empty() && req.find_header(kGhostRoleHeader).empty()) {
            req.set_header(kGhostRoleHeader, "Administrator");
            req.set_header(kGhostPermissionsHeader, "*");
        }
        bool allowed{false};
        blog::handlers::middleware::require_ghost_permission(
            req,
            res,
            [&allowed](
                interfaces::io::IRequest&,
                interfaces::io::IResponse&,
                std::function<void()>
            ) noexcept
            {
                allowed = true;
            },
            send,
            "redirects",
            "download"
        );
        if (!allowed) {
            return;
        }

        const auto ex = m_workflow.run("redirects_redirects_download_get", req);
        const auto* r = ex.result_of("redirects_redirects_download_get_execute");
        auto body = r && r->typed_output.has_value() ? std::any_cast<std::string>(r->typed_output)
                                                     : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }

        // Ghost `headers.disposition:{type:'yaml',value:'redirects.yaml'}`
        // (redirects.js:15-19) -> exact header values from headers.ts:72-78.
        res.set_header(interfaces::io::types::Token::CONTENT_TYPE, "application/yaml");
        res.set_header(
            interfaces::io::types::Token::CONTENT_DISPOSITION,
            "Attachment; filename=\"redirects.yaml\""
        );
        res.set_header(
            interfaces::io::types::Token::CONTENT_LENGTH,
            std::to_string(download_handler_detail::json_string_byte_length(body))
        );
        // Ghost cacheInvalidate:false (redirects.js:20) -> no X-Cache-Invalidate.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::redirects::redirects_download_get
