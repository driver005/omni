export module blog_operations:settings_settings_routes_yaml_get_dto;

import std;

export namespace blog::handlers::operations::settings::settings_routes_yaml_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
