export module blog_handlers:notifications_create_step;
import :domain_all;
import :notifications_service;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::notifications::create::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"notifications_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        const auto [status, body] = service::NotificationService::add(*s_ctx, in.body_as_string());
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(status)}, {"body", body}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::notifications::create::workflow
