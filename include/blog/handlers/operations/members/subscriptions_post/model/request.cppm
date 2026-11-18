export module blog_operations:members_subscriptions_post_model;

import std;

export namespace blog::handlers::operations::members::subscriptions_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
