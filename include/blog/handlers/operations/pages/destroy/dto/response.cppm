export module blog_operations:pages_destroy_dto;

import std;

export namespace blog::handlers::operations::pages::destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
