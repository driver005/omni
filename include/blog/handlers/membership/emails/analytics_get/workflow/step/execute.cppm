export module blog_handlers:emails_analytics_get_step;
import std;
import core_workflow;
import interfaces;

import blog_context;
import :emails_operations;
import blog_dto;
export namespace blog::handlers::emails::analytics_get::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"emails_analytics_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
    const auto r=s_ops->emails_analytics_get();
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  const auto one=ctx.store().find_by_resource("emails", std::string{}.empty()?"1":std::string{});
  if(one) return core::workflow::StepResponse<Output>{{{"status","200"},{"body","{\"emails\":[" + *one + "]}"}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",R"({"emails":[]})"}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::emails::EmailsOperations* s_ops;
};
blog::api::emails::EmailsOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
