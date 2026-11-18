export module blog_operations:db_bulk_destroy_dto;

import std;

export namespace blog::handlers::operations::db::bulk_destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
