export module blog_operations:notifications_create_dto;

import std;

export namespace blog::handlers::operations::notifications::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
