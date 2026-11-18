export module blog_operations:comments_browse_dto;

import std;

export namespace blog::handlers::operations::comments::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
