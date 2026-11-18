export module blog_operations:tags_create_dto;

import std;

export namespace blog::handlers::operations::tags::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
