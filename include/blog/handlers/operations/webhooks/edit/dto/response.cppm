export module blog_operations:webhooks_edit_dto;

import std;

export namespace blog::handlers::operations::webhooks::edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
