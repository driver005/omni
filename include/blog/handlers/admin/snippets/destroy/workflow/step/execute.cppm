export module blog_handlers:snippets_destroy_step;
import :domain_all;
import :snippets_browse_step;
import std;
import core_workflow;
import core_query;
import core_auth;
import interfaces;

export namespace blog::handlers::snippets::destroy::workflow {

class ExecuteStep {
public:
  using Input=interfaces::io::IRequest&;
  using Output=std::map<std::string, std::string>;
  static constexpr std::string_view name{"snippets_destroy_execute"};
  [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
   using namespace blog::handlers::snippets::support;
   auto& ctx=*s_ctx;

   core::auth::ghost::GhostAuthorizer authorizer;
   const auto authorized=authorizer.authorize(principal_for(in), true, "snippets", "destroy");
   if(!authorized.allowed)
    return core::workflow::StepResponse<Output>{{{"status",std::to_string(authorized.status)},{"body",authorized.error_json}}};

   const std::string id=path_id(in.get_path());
   if(id.empty())
    return core::workflow::StepResponse<Output>{{{"status","422"},
      {"body",blog::api::ApiError{}.validation("Validation error, cannot save snippet.","id is required","id")}}};

   seed_snippet(ctx);
   // Ghost passes require:true to Snippet.destroy -> NotFoundError on a miss.
   if(ctx.store().resources().get("snippets", id).empty())
    return core::workflow::StepResponse<Output>{{{"status","404"},{"body",not_found_error()}}};
   if(!ctx.store().resources().destroy("snippets", id))
    return core::workflow::StepResponse<Output>{{{"status","404"},{"body",not_found_error()}}};

   emit_action("deleted", id, "{}");
   return core::workflow::StepResponse<Output>{{{"status","204"},{"body",""}}};
  }
  void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
  static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
