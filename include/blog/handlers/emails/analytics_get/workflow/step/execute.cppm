export module blog_handlers:emails_analytics_get_step;
import std;
import core_workflow;
import core_cache;
import blog_context;
import :mutation;
import :emails_operations;
import blog_dto;
import :emails_analytics_get_model;
import :emails_analytics_get_dto;
export namespace blog::handlers::emails::analytics_get::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"emails_analytics_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=blog::api::emails::emails_analytics_get(*s_cache);
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  const auto one=ctx.store().find_by_resource("emails", in.id.empty()?"1":in.id);
  if(one) return core::workflow::StepResponse<Output>{{{"status","200"},{"body","{\"emails\":[" + *one + "]}"}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",R"({"emails":[]})"}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static core::cache::Cache* s_cache;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
core::cache::Cache* ExecuteStep::s_cache{nullptr};
}
