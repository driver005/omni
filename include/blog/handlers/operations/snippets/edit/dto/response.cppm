export module blog_operations:snippets_edit_dto;

import std;

export namespace blog::handlers::operations::snippets::edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
