export module blog_operations:webhooks_destroy_dto;

import std;

export namespace blog::handlers::operations::webhooks::destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
