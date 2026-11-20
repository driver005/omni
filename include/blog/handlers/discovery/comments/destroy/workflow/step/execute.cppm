export module blog_handlers:comments_destroy_step;
import :domain_all;
import :comments_model_comment;
import :comments_model_member;
import :comments_model_wire_fixtures;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::comments::destroy::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"comments_destroy_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input&, core::workflow::WorkflowContext&) const {
  return core::workflow::StepResponse<Output>{{{"status","405"},
   {"body",blog::api::ApiError{}.json("Comment deletion is not allowed","Method Not Allowed")}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
