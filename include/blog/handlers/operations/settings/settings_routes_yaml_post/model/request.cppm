export module blog_operations:settings_settings_routes_yaml_post_model;

import std;

export namespace blog::handlers::operations::settings::settings_routes_yaml_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
