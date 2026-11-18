export module blog_operations:stats_stats_subscriber_count_get_dto;

import std;

export namespace blog::handlers::operations::stats::stats_subscriber_count_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
