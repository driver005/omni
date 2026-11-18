export module blog_operations:tiers_read_by_id_dto;

import std;

export namespace blog::handlers::operations::tiers::read_by_id::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
