export module blog_operations:pages_create_model;

import std;

export namespace blog::handlers::operations::pages::create::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
