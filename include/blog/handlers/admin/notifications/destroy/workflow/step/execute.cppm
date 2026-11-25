export module blog_handlers:notifications_destroy_step;
import :domain_all;
import :notifications_service;
import std;
import core_workflow;
import interfaces;
import blog_handlers_middleware;

export namespace blog::handlers::notifications::destroy::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"notifications_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto path = std::string{in.get_path()};
        path = path.substr(0, path.find('?'));
        while (path.size() > 1 && path.back() == '/') {
            path.pop_back();
        }
        const std::string id = path.substr(path.rfind('/') + 1);
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}.validation(
                      "notification_id is required",
                      "notification_id is required",
                      "notification_id"
                  )}}
            };
        }
        const auto user_id = std::string{blog::handlers::middleware::token(in)};
        const auto [status, body] = service::NotificationService::destroy(*s_ctx, id, user_id);
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(status)}, {"body", body}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::notifications::destroy::workflow
