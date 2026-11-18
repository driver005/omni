export module blog_operations:pages_create_dto;

import std;

export namespace blog::handlers::operations::pages::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
