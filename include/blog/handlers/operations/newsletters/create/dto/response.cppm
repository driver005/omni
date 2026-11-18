export module blog_operations:newsletters_create_dto;

import std;

export namespace blog::handlers::operations::newsletters::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
