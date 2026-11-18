export module blog_operations:authors_read_by_id_dto;

import std;

export namespace blog::handlers::operations::authors::read_by_id::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
