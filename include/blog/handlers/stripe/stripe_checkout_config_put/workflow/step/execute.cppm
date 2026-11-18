export module blog_handlers:stripe_stripe_checkout_config_put_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :stripe;
import blog_dto;
import :stripe_stripe_checkout_config_put_model;
import :stripe_stripe_checkout_config_put_dto;
export namespace blog::handlers::stripe::stripe_checkout_config_put::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"stripe_stripe_checkout_config_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& state=*s_stripe;
  const auto r=[&](blog::api::StripeHandler& stripe, std::string_view method, std::string_view body, std::string_view path = "/stripe/checkout/config")->std::pair<int,std::string>{
        if (path.contains("/branding")) return {200, stripe.branding()};
        if (method == "GET") return {200, stripe.config()};
        if (method == "PUT") { stripe.configure(body); return {200, stripe.config()}; }
        if (method == "POST") return {200, stripe.preview(body)};
        return {200, stripe.branding()};
    }(*s_stripe, "PUT", in.body, in.path);
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",mutations.edit("stripe", in.id, in.body.empty()?"{}":in.body).body()}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::StripeHandler* s_stripe;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::StripeHandler* ExecuteStep::s_stripe{nullptr};
}