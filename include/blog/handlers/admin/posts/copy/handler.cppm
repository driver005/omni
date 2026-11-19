export module blog_handlers:posts_copy_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import memory_backends;
import :posts_copy_flow;

export namespace blog::handlers::posts::copy {

class PostsCopyHandler
{
public:
    explicit PostsCopyHandler(blog::BlogContext& context) noexcept :
        m_ctx{context}
    {
        workflow::ExecuteStep::m_ctx = &context;
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(interfaces::io::IRequest& request, interfaces::io::IResponse& response,
                std::function<void()> send)
    {
        const auto execution = m_workflow.run("posts_copy", request);
        const auto* result = execution.result_of("posts_copy_execute");
        const auto body = result && result->typed_output.has_value()
                              ? std::any_cast<std::string>(result->typed_output)
                              : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
        response.set_status(interfaces::io::types::Status::OK);
        response.set_body(std::move(bytes));
        send();
    }

private:
    blog::BlogContext& m_ctx;
    core::workflow::Workflow m_workflow{};
};

} // namespace
