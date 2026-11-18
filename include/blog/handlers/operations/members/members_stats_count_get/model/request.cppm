export module blog_operations:members_members_stats_count_get_model;

import std;

export namespace blog::handlers::operations::members::members_stats_count_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
