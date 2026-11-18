export module blog_operations:recommendations_edit_dto;

import std;

export namespace blog::handlers::operations::recommendations::edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
