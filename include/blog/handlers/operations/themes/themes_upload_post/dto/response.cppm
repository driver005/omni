export module blog_operations:themes_themes_upload_post_dto;

import std;

export namespace blog::handlers::operations::themes::themes_upload_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
