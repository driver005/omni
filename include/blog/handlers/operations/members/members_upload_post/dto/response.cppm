export module blog_operations:members_members_upload_post_dto;

import std;

export namespace blog::handlers::operations::members::members_upload_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
