export module blog_operations:invites_create_dto;

import std;

export namespace blog::handlers::operations::invites::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
