export module blog_handlers:comments_like_handler;
import :domain_all;
import :comments_model_comment;
import :comments_model_member;
import :comments_model_wire_fixtures;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :comments_like_flow;
export namespace blog::handlers::comments::like {
class LikeHandler {
public:
 LikeHandler(blog::BlogContext& ctx) noexcept {
  workflow::ExecuteStep::s_ctx=&ctx;
  m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
  m_workflow.register_workflow(workflow::MainFlow{}.build());
 }
 void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
  if (blog::handlers::middleware::token(req).empty()) {
   const std::string b{R"({"error":"Unauthorized"})"}; std::vector<std::byte> bytes;
   for(char c:b) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(interfaces::io::types::Status::UNAUTHORIZED); res.set_body(std::move(bytes)); send(); return;
  }
  const auto ex=m_workflow.run("comments_like", req);
  const auto* r=ex.result_of("comments_like_execute");
  const auto payload=r&&r->typed_output.has_value()
   ? std::any_cast<std::map<std::string,std::string>>(r->typed_output)
   : std::map<std::string,std::string>{{"status","204"},{"body",""}};
  const int status=payload.contains("status")?std::stoi(payload.at("status")):204;
  const auto body=payload.contains("body")?payload.at("body"):std::string{};
  const auto id=path_id(req.get_path());
  if(!id.empty()) res.set_header("X-Cache-Invalidate","/api/members/comments/"+id+"/");
  std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
  res.set_status(static_cast<interfaces::io::types::Status>(status)); res.set_body(std::move(bytes)); send();
 }
private:
 core::workflow::Workflow m_workflow{};
 static std::string path_id(std::string_view path) {
  auto bare=path.substr(0,path.find('?'));
  while(bare.size()>1 && bare.back()=='/') bare.remove_suffix(1);
  const auto slash=bare.find_last_of('/');
  return std::string{slash==std::string_view::npos ? bare : bare.substr(slash+1)};
 }
};
}
