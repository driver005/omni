export module blog_operations:stats_stats_mrr_get_model;

import std;

export namespace blog::handlers::operations::stats::stats_mrr_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
