export module blog_handlers:posts_bulk_edit_dto;

import std;

export namespace blog::handlers::posts::bulk_edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
