export module blog_handlers:webhooks_edit_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::webhooks::edit::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"webhooks_edit_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
  
  const auto body = in.body_as_string().empty() ? "{}" : in.body_as_string();
  const auto result = [&]{ const std::string p{in.get_path()}; const auto sl=p.find_last_of('/'); const auto tail=sl==std::string::npos?p:p.substr(sl+1); return (!tail.empty() && tail!="webhooks")?tail:std::string{}; }() == "bulk" ? mutations.bulk_edit("webhooks", body)
                                      : mutations.edit("webhooks", [&]{ const std::string p{in.get_path()}; const auto sl=p.find_last_of('/'); const auto tail=sl==std::string::npos?p:p.substr(sl+1); return (!tail.empty() && tail!="webhooks")?tail:std::string{}; }(), body);
  return core::workflow::StepResponse<Output>{result.body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}