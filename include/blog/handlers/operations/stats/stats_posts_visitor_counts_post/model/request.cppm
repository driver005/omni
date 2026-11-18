export module blog_operations:stats_stats_posts_visitor_counts_post_model;

import std;

export namespace blog::handlers::operations::stats::stats_posts_visitor_counts_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
