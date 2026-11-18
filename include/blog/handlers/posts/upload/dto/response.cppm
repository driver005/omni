export module blog_handlers:posts_upload_dto;

import std;

export namespace blog::handlers::posts::upload::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
