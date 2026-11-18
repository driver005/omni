export module blog_operations:authentication_authentication_setup_put_dto;

import std;

export namespace blog::handlers::operations::authentication::authentication_setup_put::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
