export module blog_handlers:files_files_upload_post_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import :files_files_upload_post_flow;

namespace blog::handlers::files::files_upload_post::detail {

/** @brief Minimal JSON string escaping for the file upload output serializer. */
inline std::string json_quote(std::string_view text)
{
    std::string out = "\"";
    for (const char c: text) {
        switch (c) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                out += c;
        }
    }
    out += '"';
    return out;
}

} // namespace blog::handlers::files::files_upload_post::detail

export namespace blog::handlers::files::files_upload_post {
class FilesUploadPostHandler
{
public:
    FilesUploadPostHandler(blog::BlogContext& ctx) noexcept
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
        // Auth is enforced by the route-level `require_auth` middleware (Ghost
        // `permissions: false`), so no in-handler token check is needed here.
        const auto ex = m_workflow.run("files_files_upload_post", req);
        const auto* r = ex.result_of("files_files_upload_post_execute");
        using Output = workflow::ExecuteStep::Output;
        const auto out =
            r && r->typed_output.has_value() ? std::any_cast<Output>(r->typed_output) : Output{};

        const auto url_it = out.find("url");
        const auto ref_it = out.find("ref");
        const std::string url = url_it == out.end() ? std::string{} : url_it->second;
        const std::string ref = ref_it == out.end() ? std::string{} : ref_it->second;

        // Ghost output serializer `files.upload`:
        //   {files:[{url: getURL(filePath), ref: frame.data.ref || null}]}
        const std::string body = std::format(
            R"({{"files":[{{"url":{},"ref":{}}}]}})",
            detail::json_quote(url),
            ref.empty() ? std::string{"null"} : detail::json_quote(ref)
        );
        // Ghost's admin app forces Cache-Control: private; headers.cacheInvalidate:false
        // suppresses X-Cache-Invalidate entirely (files.js:9-11, headers.ts:135-144).
        res.set_header("Cache-Control", "private");
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        res.set_status(interfaces::io::types::Status::CREATED);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::files::files_upload_post
