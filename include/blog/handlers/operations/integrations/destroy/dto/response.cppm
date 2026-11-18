export module blog_operations:integrations_destroy_dto;

import std;

export namespace blog::handlers::operations::integrations::destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
