export module blog_handlers:exports_exports_download_get_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :exports_exports_download_get_flow;
export namespace blog::handlers::exports::exports_download_get {

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

class ExportsDownloadGetHandler {
public:
  ExportsDownloadGetHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   if (!detail::authorize(req, res, send)) return;
   const auto ex=m_workflow.run("exports_exports_download_get", req);
   const auto* r=ex.result_of("exports_exports_download_get_execute");
   const auto payload=r&&r->typed_output.has_value()
    ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
    :std::map<std::string,std::string>{{"status","200"},{"body",""}};
   const int status=payload.contains("status")?std::stoi(payload.at("status")):200;
   const auto body=payload.contains("body")?payload.at("body"):std::string{};
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   if (const auto it=payload.find("filename"); it!=payload.end()) {
    // stream-zip-response.ts:45-59 — Content-Type, Attachment filename and
    // Cache-Control: no-transform; cacheInvalidate:false emits no X-Cache-Invalidate.
    res.set_header("Content-Type", "application/zip");
    res.set_header("Content-Disposition", "Attachment; filename=\""+it->second+"\"");
    res.set_header("Cache-Control", "no-transform");
   } else {
    res.set_header("Content-Type", "application/json");
   }
   res.set_status(static_cast<interfaces::io::types::Status>(status)); res.set_body(std::move(bytes)); send();
  }
private: core::workflow::Workflow m_workflow{};
};
}
