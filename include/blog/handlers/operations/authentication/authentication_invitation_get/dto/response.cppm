export module blog_operations:authentication_authentication_invitation_get_dto;

import std;

export namespace blog::handlers::operations::authentication::authentication_invitation_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
