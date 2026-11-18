export module blog_handlers:posts_export_dto;

import std;

export namespace blog::handlers::posts::export_op::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
