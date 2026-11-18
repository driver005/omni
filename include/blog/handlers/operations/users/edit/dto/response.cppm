export module blog_operations:users_edit_dto;

import std;

export namespace blog::handlers::operations::users::edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
