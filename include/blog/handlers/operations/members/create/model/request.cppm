export module blog_operations:members_create_model;

import std;

export namespace blog::handlers::operations::members::create::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
