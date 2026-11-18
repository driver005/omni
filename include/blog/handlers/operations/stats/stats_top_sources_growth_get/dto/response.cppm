export module blog_operations:stats_stats_top_sources_growth_get_dto;

import std;

export namespace blog::handlers::operations::stats::stats_top_sources_growth_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
