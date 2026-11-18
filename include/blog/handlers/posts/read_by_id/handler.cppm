export module blog_handlers:posts_read_by_id_handler;
import std;
import interfaces;
import blog_context;
import core_workflow;
import memory_backends;
import :posts_read_by_id_model;
import :posts_read_by_id_flow;

export namespace blog::handlers::posts::read_by_id {

class PostsReadByIdHandler
{
public:
    explicit PostsReadByIdHandler(blog::BlogContext& context) noexcept :
        m_ctx{context}
    {
        workflow::ExecuteStep::m_ctx = &context;
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(interfaces::io::IRequest& request, interfaces::io::IResponse& response,
                std::function<void()> send)
    {
        model::Request input;
        input.path = std::string{request.get_path()};
        for (const std::byte byte : request.get_body()) input.body += static_cast<char>(byte);
        const auto execution = m_workflow.run("posts_read_by_id", input);
        const auto* result = execution.result_of("posts_read_by_id_execute");
        const auto payload = result && result->typed_output.has_value()
                              ? std::any_cast<std::map<std::string, std::string>>(result->typed_output)
                              : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
        response.set_status(static_cast<interfaces::io::types::Status>(status));
        response.set_body(std::move(bytes));
        send();
    }

private:
    blog::BlogContext& m_ctx;
    core::workflow::Workflow m_workflow{};
};

} // namespace
