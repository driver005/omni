export module blog_operations:authors_browse_dto;

import std;

export namespace blog::handlers::operations::authors::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
