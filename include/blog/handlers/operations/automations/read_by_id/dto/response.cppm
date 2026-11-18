export module blog_operations:automations_read_by_id_dto;

import std;

export namespace blog::handlers::operations::automations::read_by_id::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
