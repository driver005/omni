export module blog_handlers:stripe_stripe_checkout_config_get_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :stripe;
export namespace blog::handlers::stripe::stripe_checkout_config_get::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"stripe_stripe_checkout_config_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
  auto& state=*s_stripe;
  const auto r=[&](blog::api::StripeHandler& stripe, std::string_view method, std::string_view body, std::string_view path = "/stripe/checkout/config")->std::pair<int,std::string>{
        if (path.contains("/branding")) return {200, stripe.branding()};
        if (method == "GET") return {200, stripe.config()};
        if (method == "PUT") { stripe.configure(body); return {200, stripe.config()}; }
        if (method == "POST") return {200, stripe.preview(body)};
        return {200, stripe.branding()};
    }(*s_stripe, "GET", in.body_as_string(), std::string{in.get_path()});
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  const auto rows=ctx.store().json_for("stripe", {});
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body","{\"stripe\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::StripeHandler* s_stripe;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::StripeHandler* ExecuteStep::s_stripe{nullptr};
}
