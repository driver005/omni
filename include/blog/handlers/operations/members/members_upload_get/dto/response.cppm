export module blog_operations:members_members_upload_get_dto;

import std;

export namespace blog::handlers::operations::members::members_upload_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
