export module blog_handlers:comments_get_detail_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::comments::get_detail::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"comments_get_detail_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
  
  const auto rows=ctx.store().json_for("comments", {});
  return core::workflow::StepResponse<Output>{"{\"comments\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
