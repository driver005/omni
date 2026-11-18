export module blog_operations:emails_analytics_get_model;

import std;

export namespace blog::handlers::operations::emails::analytics_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
