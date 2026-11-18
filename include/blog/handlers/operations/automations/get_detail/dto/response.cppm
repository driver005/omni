export module blog_operations:automations_get_detail_dto;

import std;

export namespace blog::handlers::operations::automations::get_detail::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
