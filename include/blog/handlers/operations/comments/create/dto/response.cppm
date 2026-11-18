export module blog_operations:comments_create_dto;

import std;

export namespace blog::handlers::operations::comments::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
