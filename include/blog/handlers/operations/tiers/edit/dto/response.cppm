export module blog_operations:tiers_edit_dto;

import std;

export namespace blog::handlers::operations::tiers::edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
