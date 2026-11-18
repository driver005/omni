export module blog_operations:themes_destroy_dto;

import std;

export namespace blog::handlers::operations::themes::destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
