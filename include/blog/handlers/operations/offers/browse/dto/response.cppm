export module blog_operations:offers_browse_dto;

import std;

export namespace blog::handlers::operations::offers::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
