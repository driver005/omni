export module blog_operations:notifications_browse_dto;

import std;

export namespace blog::handlers::operations::notifications::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
