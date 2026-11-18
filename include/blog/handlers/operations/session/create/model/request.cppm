export module blog_operations:session_create_model;

import std;

export namespace blog::handlers::operations::session::create::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
