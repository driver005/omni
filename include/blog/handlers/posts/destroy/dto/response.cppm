export module blog_handlers:posts_destroy_dto;

import std;

export namespace blog::handlers::posts::destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
