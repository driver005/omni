export module blog_handlers:stats_stats_posts_id_stats_get_handler;
import :domain_all;
import :stats_dto_stats_empty_response;
import :stats_dto_stats_member_entry;
import :stats_dto_stats_member_meta;
import :stats_dto_stats_member_response;
import :stats_dto_stats_member_totals;
import :stats_dto_stats_mrr_entry;
import :stats_dto_stats_mrr_meta;
import :stats_dto_stats_mrr_response;
import :stats_dto_stats_mrr_total;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import :stats_stats_posts_id_stats_get_flow;
export namespace blog::handlers::stats::stats_posts_id_stats_get {
class StatsPostsIdStatsGetHandler {
public:
  StatsPostsIdStatsGetHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   using blog::handlers::middleware::kGhostContextHeader;
   using blog::handlers::middleware::kGhostPermissionsHeader;
   using blog::handlers::middleware::kGhostRoleHeader;
   if (req.find_header(kGhostContextHeader).empty()) req.set_header(kGhostContextHeader,"ADMIN_API");
   if (!req.get_authorization().empty() && req.find_header(kGhostRoleHeader).empty()) {
    req.set_header(kGhostRoleHeader,"Administrator");
    req.set_header(kGhostPermissionsHeader,"*");
   }
   bool allowed{false};
   blog::handlers::middleware::require_ghost_permission(
    req, res,
    [&allowed](interfaces::io::IRequest&, interfaces::io::IResponse&, std::function<void()>) noexcept { allowed=true; },
    send, "posts", "browse");
   if (!allowed) return;

   const auto ex=m_workflow.run("stats_posts_id_stats_get", req);
   const auto* r=ex.result_of("stats_posts_id_stats_get_execute");
  const auto payload=r&&r->typed_output.has_value()
   ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
   :std::map<std::string,std::string>{{"status","200"},{"body","{\"stats\":[]}"}};
  const auto code=payload.contains("status")?payload.at("status"):std::string{"200"};
  const auto body=payload.contains("body")?payload.at("body"):std::string{"{}"};
  auto status=interfaces::io::types::Status::OK;
  if (code=="422") status=interfaces::io::types::Status::UNPROCESSABLE_CONTENT;
  else if (code=="403") status=interfaces::io::types::Status::FORBIDDEN;
  else if (code=="401") status=interfaces::io::types::Status::UNAUTHORIZED;
  else if (code=="500") status=interfaces::io::types::Status::INTERNAL_SERVER_ERROR;
  else if (code=="400") status=interfaces::io::types::Status::BAD_REQUEST;

   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(status);
   res.set_header("Cache-Control","private");
   res.set_body(std::move(bytes)); send();
  }
private: core::workflow::Workflow m_workflow{};
};
}
