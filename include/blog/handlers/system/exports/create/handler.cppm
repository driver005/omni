export module blog_handlers:exports_create_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :exports_create_flow;
export namespace blog::handlers::exports::create {

namespace detail {

/** @brief Stand-in for Ghost's `authAdminApi` identity step. */
inline void prepare_principal(interfaces::io::IRequest& req)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostRoleHeader;
    using blog::handlers::middleware::kGhostPermissionsHeader;
    if (req.get_authorization().empty()) return;
    if (req.find_header(kGhostContextHeader).empty())
        req.set_header(kGhostContextHeader, "ADMIN_API");
    if (req.find_header(kGhostRoleHeader).empty())
        req.set_header(kGhostRoleHeader, "Administrator");
    if (req.find_header(kGhostPermissionsHeader).empty())
        req.set_header(kGhostPermissionsHeader, "*");
}

/** @brief Ghost controller permission override `{docName:'db', method:'exportContent'}`. */
[[nodiscard]] inline bool authorize(interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send)
{
    using blog::handlers::middleware::GhostPermissionRegistry;
    prepare_principal(req);
    bool denied = false;
    auto sentinel = [&denied, send = std::move(send)]() mutable {
        denied = true;
        send();
    };
    GhostPermissionRegistry::set_active(&req, &res, sentinel);
    blog::handlers::middleware::require_ghost_permission("db", "exportContent");
    GhostPermissionRegistry::clear_active();
    return !denied;
}

} // namespace detail

class CreateHandler {
public:
 CreateHandler(blog::BlogContext& ctx) noexcept {
  workflow::ExecuteStep::s_ctx=&ctx;
  m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
  m_workflow.register_workflow(workflow::MainFlow{}.build());
 }
 void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
  if (!detail::authorize(req, res, send)) return;
  const auto ex=m_workflow.run("exports_create", req);
  const auto* r=ex.result_of("exports_create_execute");
  const auto payload=r&&r->typed_output.has_value()
   ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
   :std::map<std::string,std::string>{{"status","202"},{"body","{}"}};
  const int status=payload.contains("status")?std::stoi(payload.at("status")):202;
  const auto body=payload.contains("body")?payload.at("body"):std::string{"{}"};
  std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
  // Ghost `headers.cacheInvalidate:false` -> no X-Cache-Invalidate header.
  res.set_status(static_cast<interfaces::io::types::Status>(status)); res.set_body(std::move(bytes)); send();
 }
private: core::workflow::Workflow m_workflow{};
};
}
