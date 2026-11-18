export module blog_operations:members_browse_dto;

import std;

export namespace blog::handlers::operations::members::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
