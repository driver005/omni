export module blog_operations:authentication_authentication_setup_post_dto;

import std;

export namespace blog::handlers::operations::authentication::authentication_setup_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
