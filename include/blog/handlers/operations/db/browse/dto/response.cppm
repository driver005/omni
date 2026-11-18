export module blog_operations:db_browse_dto;

import std;

export namespace blog::handlers::operations::db::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
