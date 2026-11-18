export module blog_operations:members_signin_urls_dto;

import std;

export namespace blog::handlers::operations::members::signin_urls::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
