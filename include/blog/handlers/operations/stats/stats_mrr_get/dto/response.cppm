export module blog_operations:stats_stats_mrr_get_dto;

import std;

export namespace blog::handlers::operations::stats::stats_mrr_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
