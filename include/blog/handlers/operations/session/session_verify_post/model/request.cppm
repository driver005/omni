export module blog_operations:session_session_verify_post_model;

import std;

export namespace blog::handlers::operations::session::session_verify_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
