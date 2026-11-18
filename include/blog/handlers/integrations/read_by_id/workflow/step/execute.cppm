export module blog_handlers:integrations_read_by_id_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :special;
import blog_dto;
import :integrations_read_by_id_model;
import :integrations_read_by_id_dto;
export namespace blog::handlers::integrations::read_by_id::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"integrations_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  const auto r=special.dispatch(std::string_view{in.path}.substr(0, in.path.find('?')), "GET", in.id, in.body);
  if(r.status()!=0) return core::workflow::StepResponse<Output>{r.body()};
  const auto one=ctx.store().find_by_resource("integrations", in.id.empty()?"1":in.id);
  if(one) return core::workflow::StepResponse<Output>{"{\"integrations\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"integrations":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}
