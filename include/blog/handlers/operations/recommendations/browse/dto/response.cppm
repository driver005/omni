export module blog_operations:recommendations_browse_dto;

import std;

export namespace blog::handlers::operations::recommendations::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
