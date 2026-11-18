export module blog_operations:automations_create_dto;

import std;

export namespace blog::handlers::operations::automations::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
