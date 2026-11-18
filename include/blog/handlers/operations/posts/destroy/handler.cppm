export module blog_operations:posts_destroy_handler;

import std;
import interfaces;
import blog_handlers_runtime;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :posts_destroy_model;
import :posts_destroy_flow;

export namespace blog::handlers::operations::posts::destroy {

class DestroyPostsHandler
{
public:
    static void handle(
        interfaces::io::IRequest& request,
        interfaces::io::IResponse& response,
        std::function<void()> send)
    {
        model::RequestData input;
        const auto path = std::string{request.get_path()};
        const auto slash = path.find('/');
        if (slash != std::string::npos) input.id = path.substr(slash + 1);
        for (const std::byte byte : request.get_body()) input.body += static_cast<char>(byte);

        core::workflow::Workflow workflow;
        workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        workflow.register_workflow(workflow::MainFlow{}.build());
        const auto execution = workflow.run("posts_destroy", input);
        const auto* result = execution.result_of("posts_destroy_execute");
        auto body = result && result->typed_output.has_value()
                        ? std::any_cast<std::string>(result->typed_output)
                        : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
        std::move(response).with_status(interfaces::io::types::Status::OK).with_body(std::move(bytes));
        send();
    }
};

} // namespace
