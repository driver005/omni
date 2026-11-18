export module blog_operations:newsletters_browse_dto;

import std;

export namespace blog::handlers::operations::newsletters::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
