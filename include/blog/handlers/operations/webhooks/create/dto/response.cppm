export module blog_operations:webhooks_create_dto;

import std;

export namespace blog::handlers::operations::webhooks::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
