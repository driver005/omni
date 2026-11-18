export module blog_operations:automations_automations_id_performance_stats_get_model;

import std;

export namespace blog::handlers::operations::automations::automations_id_performance_stats_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
