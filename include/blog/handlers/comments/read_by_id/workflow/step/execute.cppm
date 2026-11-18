export module blog_handlers:comments_read_by_id_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :special;
import blog_dto;
import :comments_read_by_id_model;
import :comments_read_by_id_dto;
export namespace blog::handlers::comments::read_by_id::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"comments_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  const auto r=special.dispatch(std::string_view{in.path}.substr(0, in.path.find('?')), "GET", in.id, in.body);
  if(r.status()!=0) return core::workflow::StepResponse<Output>{r.body()};
  const auto one=ctx.store().find_by_resource("comments", in.id.empty()?"1":in.id);
  if(one) return core::workflow::StepResponse<Output>{"{\"comments\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"comments":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}
