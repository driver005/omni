export module blog_operations:offers_edit_dto;

import std;

export namespace blog::handlers::operations::offers::edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
