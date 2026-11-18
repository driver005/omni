export module blog_operations:exports_create_dto;

import std;

export namespace blog::handlers::operations::exports::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
