export module blog_operations:comments_create_model;

import std;

export namespace blog::handlers::operations::comments::create::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
