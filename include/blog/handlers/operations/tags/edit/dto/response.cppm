export module blog_operations:tags_edit_dto;

import std;

export namespace blog::handlers::operations::tags::edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
