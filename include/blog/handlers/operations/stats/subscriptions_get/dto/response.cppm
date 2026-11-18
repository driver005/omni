export module blog_operations:stats_subscriptions_get_dto;

import std;

export namespace blog::handlers::operations::stats::subscriptions_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
