export module blog_operations:notifications_destroy_dto;

import std;

export namespace blog::handlers::operations::notifications::destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
