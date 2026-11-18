export module blog_operations:users_read_by_slug_dto;

import std;

export namespace blog::handlers::operations::users::read_by_slug::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
