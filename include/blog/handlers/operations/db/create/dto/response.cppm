export module blog_operations:db_create_dto;

import std;

export namespace blog::handlers::operations::db::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
