export module blog_handlers:posts_bulk_destroy_dto;

import std;

export namespace blog::handlers::posts::bulk_destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
