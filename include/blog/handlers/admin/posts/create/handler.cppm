export module blog_handlers:posts_create_handler;
import :domain_all;
import :posts_support;
import std;
import interfaces;
import core_workflow;
import blog_handlers_middleware;
import memory_backends;
import :posts_create_flow;

export namespace blog::handlers::posts::create {

class PostsCreateHandler
{
public:
    explicit PostsCreateHandler(blog::BlogContext& context) noexcept :
        m_ctx{context}
    {
        workflow::ExecuteStep::s_ctx = &context;
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(interfaces::io::IRequest& request, interfaces::io::IResponse& response,
                std::function<void()> send)
    {
        if (!blog::handlers::posts::support::authorize(
                request, response, send, "posts", "add")) {
            return;
        }
        const auto execution = m_workflow.run("posts_create", request);
        const auto* result = execution.result_of("posts_create_execute");
        const auto payload = result && result->typed_output.has_value()
                               ? std::any_cast<std::map<std::string, std::string>>(result->typed_output)
                               : std::map<std::string, std::string>{{"status", "201"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 201;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
        if (payload.contains("published") && payload.at("published") == "true") {
            response.set_header("X-Cache-Invalidate", "/*");
        }
        if (payload.contains("location") && !payload.at("location").empty()) {
            response.set_header(interfaces::io::types::Token::LOCATION, payload.at("location"));
        }
        response.set_status(static_cast<interfaces::io::types::Status>(status));
        response.set_body(std::move(bytes));
        send();
    }

private:
    blog::BlogContext& m_ctx;
    core::workflow::Workflow m_workflow{};
};

} // namespace
