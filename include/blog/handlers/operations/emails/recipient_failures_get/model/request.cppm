export module blog_operations:emails_recipient_failures_get_model;

import std;

export namespace blog::handlers::operations::emails::recipient_failures_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
