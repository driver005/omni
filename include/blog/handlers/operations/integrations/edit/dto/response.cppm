export module blog_operations:integrations_edit_dto;

import std;

export namespace blog::handlers::operations::integrations::edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
