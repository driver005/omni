export module blog_operations:members_bulk_destroy_dto;

import std;

export namespace blog::handlers::operations::members::bulk_destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
