export module blog_operations:tiers_create_dto;

import std;

export namespace blog::handlers::operations::tiers::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
