export module blog_operations:snippets_create_dto;

import std;

export namespace blog::handlers::operations::snippets::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
