export module blog_operations:members_subscriptions_post_dto;

import std;

export namespace blog::handlers::operations::members::subscriptions_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
