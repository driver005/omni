export module blog_operations:emails_analytics_delete_model;

import std;

export namespace blog::handlers::operations::emails::analytics_delete::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
