export module blog_operations:users_users_password_put_dto;

import std;

export namespace blog::handlers::operations::users::users_password_put::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
