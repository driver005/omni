export module blog_operations:members_put_detail_dto;

import std;

export namespace blog::handlers::operations::members::put_detail::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
