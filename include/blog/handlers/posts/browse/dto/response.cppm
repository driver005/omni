export module blog_handlers:posts_browse_dto;

import std;

export namespace blog::handlers::posts::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
