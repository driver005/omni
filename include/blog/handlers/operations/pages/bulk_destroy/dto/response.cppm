export module blog_operations:pages_bulk_destroy_dto;

import std;

export namespace blog::handlers::operations::pages::bulk_destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
