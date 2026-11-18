export module blog_operations:automations_browse_dto;

import std;

export namespace blog::handlers::operations::automations::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
