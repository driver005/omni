export module blog_operations:session_session_verify_post_dto;

import std;

export namespace blog::handlers::operations::session::session_verify_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
