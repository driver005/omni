export module blog_operations:invites_destroy_dto;

import std;

export namespace blog::handlers::operations::invites::destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
