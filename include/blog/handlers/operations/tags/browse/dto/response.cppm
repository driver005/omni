export module blog_operations:tags_browse_dto;

import std;

export namespace blog::handlers::operations::tags::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
