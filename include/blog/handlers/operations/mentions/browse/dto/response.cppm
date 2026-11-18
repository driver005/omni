export module blog_operations:mentions_browse_dto;

import std;

export namespace blog::handlers::operations::mentions::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
