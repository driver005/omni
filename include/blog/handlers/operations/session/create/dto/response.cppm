export module blog_operations:session_create_dto;

import std;

export namespace blog::handlers::operations::session::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
