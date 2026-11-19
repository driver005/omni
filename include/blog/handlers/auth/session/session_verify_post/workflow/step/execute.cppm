export module blog_handlers:session_session_verify_post_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::session::session_verify_post::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"session_session_verify_post_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
  
  return core::workflow::StepResponse<Output>{mutations.create("session", in.body_as_string().empty()?"{}":in.body_as_string()).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}