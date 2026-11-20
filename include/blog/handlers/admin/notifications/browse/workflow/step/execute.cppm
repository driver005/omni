export module blog_handlers:notifications_browse_step;
import :domain_all;
import :notifications_service;
import std;
import core_workflow;
import interfaces;
import blog_handlers_middleware;

export namespace blog::handlers::notifications::browse::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::string;
 static constexpr std::string_view name{"notifications_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const auto path=std::string{in.get_path()};
  const auto user_id=std::string{blog::handlers::middleware::token(in)};
  return core::workflow::StepResponse<Output>{
   service::NotificationService::browse(ctx, path, user_id)};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
