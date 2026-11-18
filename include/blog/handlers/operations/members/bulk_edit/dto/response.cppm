export module blog_operations:members_bulk_edit_dto;

import std;

export namespace blog::handlers::operations::members::bulk_edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
