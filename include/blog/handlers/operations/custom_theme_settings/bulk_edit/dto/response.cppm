export module blog_operations:custom_theme_settings_bulk_edit_dto;

import std;

export namespace blog::handlers::operations::custom_theme_settings::bulk_edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
