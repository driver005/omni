export module blog_operations:members_edit_handler;

import std;
import interfaces;
import blog_handlers_runtime;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :members_edit_model;
import :members_edit_flow;

export namespace blog::handlers::operations::members::edit {

class EditMembersHandler
{
public:
    static void handle(
        interfaces::io::IRequest& request,
        interfaces::io::IResponse& response,
        std::function<void()> send)
    {
        model::RequestData input;
        input.path = std::string{request.get_path()};
        input.token = std::string{blog::handlers::middleware::token(request)};
        const auto q = input.path.find('?');
        if (q != std::string::npos) input.path.erase(q);
        for (const std::byte byte : request.get_body()) input.body += static_cast<char>(byte);
        if (input.token.empty()) {
            const std::string body{R"({"error":"Unauthorized"})"};
            std::vector<std::byte> bytes;
            for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
            std::move(response).with_status(interfaces::io::types::Status::UNAUTHORIZED).with_body(std::move(bytes));
            send();
            return;
        }
        const auto parts = split(input.path);
        if (parts.size() > 1) input.id = parts[1];
        if (parts.size() > 2) input.sub = parts[2];

        core::workflow::Workflow workflow;
        workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        workflow.register_workflow(workflow::MainFlow{}.build());
        const auto execution = workflow.run("members_edit", input);
        const auto* result = execution.result_of("members_edit_execute");
        auto values = result && result->typed_output.has_value()
                          ? std::any_cast<std::map<std::string, std::string>>(result->typed_output)
                          : std::map<std::string, std::string>{{"status","500"},{"body","{}"}};
        const int status = values.contains("status") ? std::stoi(values["status"]) : 200;
        const auto body = values.contains("body") ? values["body"] : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
        std::move(response)
            .with_status(static_cast<interfaces::io::types::Status>(status))
            .with_body(std::move(bytes));
        send();
    }

private:
    static std::vector<std::string> split(std::string_view path)
    {
        std::vector<std::string> out;
        std::size_t start = 0;
        while (start < path.size()) {
            const auto slash = path.find('/', start);
            const auto stop = slash == std::string_view::npos ? path.size() : slash;
            if (stop > start) out.emplace_back(path.substr(start, stop - start));
            if (slash == std::string_view::npos) break;
            start = slash + 1;
        }
        return out;
    }
};

} // namespace
