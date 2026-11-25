export module blog_handlers:feedback_add_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :feedback_add_flow;

export namespace blog::handlers::feedback::add {
class AddHandler
{
public:
    AddHandler(blog::BlogContext& ctx) noexcept
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
        const auto ex = m_workflow.run("feedback_add", req);
        const auto* r = ex.result_of("feedback_add_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"status", "201"}, {"body", "{}"}};
        const std::string code = payload.contains("status") ? payload.at("status") : "201";
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        // controller statusCode:201 — every successful add/edit answers 201.
        auto status = interfaces::io::types::Status::CREATED;
        if (code == "422") {
            status = interfaces::io::types::Status::UNPROCESSABLE_CONTENT;
        } else if (code == "404") {
            status = interfaces::io::types::Status::NOT_FOUND;
        } else if (code == "500") {
            status = interfaces::io::types::Status::INTERNAL_SERVER_ERROR;
        }
        // controller headers.cacheInvalidate:false -> X-Cache-Invalidate:false.
        res.set_header("X-Cache-Invalidate", "false");
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::feedback::add
