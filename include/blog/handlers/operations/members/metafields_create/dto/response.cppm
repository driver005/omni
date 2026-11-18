export module blog_operations:members_metafields_create_dto;

import std;

export namespace blog::handlers::operations::members::metafields_create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
