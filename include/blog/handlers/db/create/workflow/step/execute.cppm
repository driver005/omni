export module blog_handlers:db_create_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :special;
import blog_dto;
import :db_create_model;
import :db_create_dto;
export namespace blog::handlers::db::create::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"db_create_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  
  return core::workflow::StepResponse<Output>{mutations.create("db", in.body.empty()?"{}":in.body).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}