export module blog_operations:snippets_read_by_id_dto;

import std;

export namespace blog::handlers::operations::snippets::read_by_id::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
