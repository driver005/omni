export module blog_operations:custom_theme_settings_browse_dto;

import std;

export namespace blog::handlers::operations::custom_theme_settings::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
