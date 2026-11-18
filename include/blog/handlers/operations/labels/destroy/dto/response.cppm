export module blog_operations:labels_destroy_dto;

import std;

export namespace blog::handlers::operations::labels::destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
