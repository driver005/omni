export module blog_operations:session_bulk_destroy_dto;

import std;

export namespace blog::handlers::operations::session::bulk_destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
