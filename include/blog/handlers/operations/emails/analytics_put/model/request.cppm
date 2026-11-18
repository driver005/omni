export module blog_operations:emails_analytics_put_model;

import std;

export namespace blog::handlers::operations::emails::analytics_put::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
