export module blog_operations:labels_edit_dto;

import std;

export namespace blog::handlers::operations::labels::edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
