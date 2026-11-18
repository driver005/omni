export module blog_operations:labels_browse_dto;

import std;

export namespace blog::handlers::operations::labels::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
