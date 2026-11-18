export module blog_operations:integrations_create_dto;

import std;

export namespace blog::handlers::operations::integrations::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
