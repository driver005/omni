export module blog_operations:users_read_by_email_dto;

import std;

export namespace blog::handlers::operations::users::read_by_email::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
