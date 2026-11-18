export module blog_operations:snippets_browse_dto;

import std;

export namespace blog::handlers::operations::snippets::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
