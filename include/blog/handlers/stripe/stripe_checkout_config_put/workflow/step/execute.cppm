export module blog_handlers:stripe_stripe_checkout_config_put_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :stripe;
import :special;
import blog_dto;
import :stripe_stripe_checkout_config_put_model;
import :stripe_stripe_checkout_config_put_dto;
export namespace blog::handlers::stripe::stripe_checkout_config_put::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"stripe_stripe_checkout_config_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  const auto r=special.dispatch(std::string_view{in.path}.substr(0, in.path.find('?')), "PUT", in.id, in.body);
  if(r.status()!=0) return core::workflow::StepResponse<Output>{r.body()};
  return core::workflow::StepResponse<Output>{mutations.edit("stripe", in.id, in.body.empty()?"{}":in.body).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}