export module blog_operations:newsletters_edit_dto;

import std;

export namespace blog::handlers::operations::newsletters::edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
