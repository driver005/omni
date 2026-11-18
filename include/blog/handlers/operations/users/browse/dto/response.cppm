export module blog_operations:users_browse_dto;

import std;

export namespace blog::handlers::operations::users::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
