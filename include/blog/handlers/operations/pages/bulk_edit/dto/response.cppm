export module blog_operations:pages_bulk_edit_dto;

import std;

export namespace blog::handlers::operations::pages::bulk_edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
