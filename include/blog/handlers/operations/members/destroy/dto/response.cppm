export module blog_operations:members_destroy_dto;

import std;

export namespace blog::handlers::operations::members::destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
