export module blog_operations:comments_edit_dto;

import std;

export namespace blog::handlers::operations::comments::edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
