export module blog_operations:oembed_browse_dto;

import std;

export namespace blog::handlers::operations::oembed::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
