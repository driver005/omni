export module blog_operations:links_browse_dto;

import std;

export namespace blog::handlers::operations::links::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
