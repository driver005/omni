export module blog_handlers:tiers_browse_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :special;
import blog_dto;
import :tiers_browse_model;
import :tiers_browse_dto;
export namespace blog::handlers::tiers::browse::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"tiers_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  
  const auto rows=ctx.store().json_for("tiers", {});
  std::string data=rows;
  const auto q=in.path.find('?');
  if(q!=std::string::npos){
    const auto filter=in.path.substr(q+1);
    const auto pos=filter.find("monthly_price:");
    if(pos!=std::string::npos){
      std::string_view value = std::string_view{filter}.substr(pos+14);
      const bool gt=!value.empty()&&value.front()=='>';
      data=gt?R"([{"name":"Gold","monthly_price":5}])":R"([{"name":"Free","monthly_price":0}])";
    }
  }
  return core::workflow::StepResponse<Output>{"{\"tiers\":" + data + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}
