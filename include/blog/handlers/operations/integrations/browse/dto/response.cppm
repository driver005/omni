export module blog_operations:integrations_browse_dto;

import std;

export namespace blog::handlers::operations::integrations::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
