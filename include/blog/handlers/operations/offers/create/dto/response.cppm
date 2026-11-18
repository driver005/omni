export module blog_operations:offers_create_dto;

import std;

export namespace blog::handlers::operations::offers::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
