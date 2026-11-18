export module blog_operations:newsletters_create_model;

import std;

export namespace blog::handlers::operations::newsletters::create::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
