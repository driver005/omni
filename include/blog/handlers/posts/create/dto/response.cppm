export module blog_handlers:posts_create_dto;

import std;

export namespace blog::handlers::posts::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
