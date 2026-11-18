export module blog_operations:themes_themes_name_download_get_dto;

import std;

export namespace blog::handlers::operations::themes::themes_name_download_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
