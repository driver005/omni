export module blog_operations:notifications_create_model;

import std;

export namespace blog::handlers::operations::notifications::create::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
