export module blog_operations:themes_themes_active_get_dto;

import std;

export namespace blog::handlers::operations::themes::themes_active_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
