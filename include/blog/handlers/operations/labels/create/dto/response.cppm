export module blog_operations:labels_create_dto;

import std;

export namespace blog::handlers::operations::labels::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
