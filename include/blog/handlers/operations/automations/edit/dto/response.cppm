export module blog_operations:automations_edit_dto;

import std;

export namespace blog::handlers::operations::automations::edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
