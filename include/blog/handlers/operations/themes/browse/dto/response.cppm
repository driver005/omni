export module blog_operations:themes_browse_dto;

import std;

export namespace blog::handlers::operations::themes::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
