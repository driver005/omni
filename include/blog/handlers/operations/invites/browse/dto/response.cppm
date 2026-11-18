export module blog_operations:invites_browse_dto;

import std;

export namespace blog::handlers::operations::invites::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
