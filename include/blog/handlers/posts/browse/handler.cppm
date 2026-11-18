export module blog_handlers_posts_browse;

import std;
import interfaces;
import blog_context;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :model_query_input;
import :dto_browse_response;
import :dto_errors;
import :flow_browse;

export namespace blog::handlers::posts::browse {

/**
 * @brief Self-contained GET /posts handler. Owns its model, DTOs, workflow and
 * middleware registration; no CoreHandler dependency.
 */
class PostsBrowseHandler
{
public:
    explicit PostsBrowseHandler(blog::BlogContext& context) :
        m_context{context}
    {
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::BrowsePostsFlow{}.build());
    }

    void handle(
        interfaces::io::IRequest& request,
        interfaces::io::IResponse& response,
        std::function<void()> send
    )
    {
        const auto raw = std::string{request.get_path()};
        const auto execution = m_workflow.run("posts_browse", raw);
        if (!execution.succeeded()) {
            middleware::write_error(
                response,
                interfaces::io::types::Status::INTERNAL_SERVER_ERROR,
                dto::not_found(),
                std::move(send)
            );
            return;
        }
        const auto* result = execution.result_of("paginate");
        if (result == nullptr || !result->typed_output.has_value()) {
            middleware::write_error(
                response,
                interfaces::io::types::Status::INTERNAL_SERVER_ERROR,
                dto::not_found(),
                std::move(send)
            );
            return;
        }
        const auto body = std::any_cast<std::string>(result->typed_output);
        std::vector<std::byte> bytes;
        bytes.reserve(body.size());
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
        std::move(response)
            .with_status(interfaces::io::types::Status::OK)
            .with_body(std::move(bytes));
        send();
    }

    blog::BlogContext& context() noexcept
    {
        return m_context;
    }

private:
    blog::BlogContext& m_context;
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::posts::browse
