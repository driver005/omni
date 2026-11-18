export module blog_operations:users_destroy_dto;

import std;

export namespace blog::handlers::operations::users::destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
