export module blog_handlers:featurebase_featurebase_token_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :featurebase_featurebase_token_get_step;
export namespace blog::handlers::featurebase::featurebase_token_get::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("featurebase_featurebase_token_get").step<ExecuteStep>().build();
 }};
}
