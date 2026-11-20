export module blog_handlers:comments_dislike_step;
import :domain_all;
import :comments_model_comment;
import :comments_model_member;
import :comments_model_wire_fixtures;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::comments::dislike::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"comments_dislike_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const auto id=last_segment(in.get_path());
  if(id.empty()) return invalid("id");
  const auto existing=ctx.store().json_for("comments", std::map<std::string,std::string>{{"filter[id]",id}});
  if(existing=="[]") return missing();
  ctx.store().resources().create("comment_likes",
   "{\"comment_id\":\"" + id + "\",\"member_id\":\"" + member(in) + "\",\"score\":-1}");
  return core::workflow::StepResponse<Output>{{{"status","204"},{"body",""}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static std::string last_segment(std::string_view path) {
  auto bare=path.substr(0,path.find('?'));
  while(bare.size()>1 && bare.back()=='/') bare.remove_suffix(1);
  const auto slash=bare.find_last_of('/');
  return std::string{slash==std::string_view::npos ? bare : bare.substr(slash+1)};
 }
 static std::string member(const Input& in) {
  const auto header=in.find_header("X-Member-Id");
  return header.empty()?"1":std::string{header};
 }
 static core::workflow::StepResponse<Output> invalid(std::string_view property) {
  return core::workflow::StepResponse<Output>{{{"status","422"},
   {"body",blog::api::ApiError{}.validation("Validation error","Missing required option",property)}}};
 }
 static core::workflow::StepResponse<Output> missing() {
  return core::workflow::StepResponse<Output>{{{"status","404"},
   {"body",blog::api::ApiError{}.json("Resource not found","Not Found")}}};
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
