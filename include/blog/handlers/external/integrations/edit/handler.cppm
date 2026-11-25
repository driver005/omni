export module blog_handlers:integrations_edit_handler;
import :domain_all;
import :integrations_dto_more_resources;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :integrations_edit_flow;

export namespace blog::handlers::integrations::edit {
class EditHandler
{
public:
    EditHandler(blog::BlogContext& ctx) noexcept
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
        namespace support = blog::handlers::integrations::support;
        if (!support::authorized(req, res, send, "edit")) {
            return;
        }
        const auto ex = m_workflow.run("integrations_edit", req);
        const auto* r = ex.result_of("integrations_edit_execute");
        auto payload = r && r->typed_output.has_value()
                           ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                           : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const std::string code = payload.contains("status") ? payload.at("status") : "200";
        const std::string body = payload.contains("body") ? payload.at("body") : "{}";
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        auto status = interfaces::io::types::Status::OK;
        if (code == "404") {
            status = interfaces::io::types::Status::NOT_FOUND;
        } else if (code == "422") {
            status = interfaces::io::types::Status::UNPROCESSABLE_CONTENT;
        }
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::integrations::edit
