export module blog_handlers:redirects_redirects_download_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :redirects_redirects_download_get_step;
export namespace blog::handlers::redirects::redirects_download_get::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("redirects_redirects_download_get").step<ExecuteStep>().build();
 }};
}
