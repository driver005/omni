export module blog_operations:pages_browse_dto;

import std;

export namespace blog::handlers::operations::pages::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
