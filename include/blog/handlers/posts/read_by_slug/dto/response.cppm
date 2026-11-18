export module blog_handlers:posts_read_by_slug_dto;

import std;

export namespace blog::handlers::posts::read_by_slug::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
