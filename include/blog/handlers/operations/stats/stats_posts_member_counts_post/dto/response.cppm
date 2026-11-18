export module blog_operations:stats_stats_posts_member_counts_post_dto;

import std;

export namespace blog::handlers::operations::stats::stats_posts_member_counts_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
