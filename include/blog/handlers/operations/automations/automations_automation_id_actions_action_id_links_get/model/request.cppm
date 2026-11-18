export module blog_operations:automations_automations_automation_id_actions_action_id_links_get_model;

import std;

export namespace blog::handlers::operations::automations::automations_automation_id_actions_action_id_links_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
