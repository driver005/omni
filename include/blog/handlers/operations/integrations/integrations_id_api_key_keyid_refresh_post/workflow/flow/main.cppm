export module blog_operations:integrations_integrations_id_api_key_keyid_refresh_post_flow;

import std;
import core_workflow;
import :integrations_integrations_id_api_key_keyid_refresh_post_step;

export namespace blog::handlers::operations::integrations::integrations_id_api_key_keyid_refresh_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("integrations_integrations_id_api_key_keyid_refresh_post").step<ExecuteStep>().build();
    }
};

} // namespace
