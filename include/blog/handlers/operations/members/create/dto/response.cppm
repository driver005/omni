export module blog_operations:members_create_dto;

import std;

export namespace blog::handlers::operations::members::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
