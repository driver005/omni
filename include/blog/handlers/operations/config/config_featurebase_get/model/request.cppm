export module blog_operations:config_config_featurebase_get_model;

import std;

export namespace blog::handlers::operations::config::config_featurebase_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
