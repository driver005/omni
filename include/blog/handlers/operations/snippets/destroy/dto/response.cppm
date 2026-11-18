export module blog_operations:snippets_destroy_dto;

import std;

export namespace blog::handlers::operations::snippets::destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
