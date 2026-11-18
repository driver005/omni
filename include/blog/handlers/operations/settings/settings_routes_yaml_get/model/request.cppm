export module blog_operations:settings_settings_routes_yaml_get_model;

import std;

export namespace blog::handlers::operations::settings::settings_routes_yaml_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
