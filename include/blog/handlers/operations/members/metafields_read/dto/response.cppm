export module blog_operations:members_metafields_read_dto;

import std;

export namespace blog::handlers::operations::members::metafields_read::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
