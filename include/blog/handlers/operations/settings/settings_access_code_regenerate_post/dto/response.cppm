export module blog_operations:settings_settings_access_code_regenerate_post_dto;

import std;

export namespace blog::handlers::operations::settings::settings_access_code_regenerate_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
