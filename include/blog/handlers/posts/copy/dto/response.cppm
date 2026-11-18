export module blog_handlers:posts_copy_dto;

import std;

export namespace blog::handlers::posts::copy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
