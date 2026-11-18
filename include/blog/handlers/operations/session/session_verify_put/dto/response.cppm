export module blog_operations:session_session_verify_put_dto;

import std;

export namespace blog::handlers::operations::session::session_verify_put::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
