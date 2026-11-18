export module blog_operations:tiers_browse_model;

import std;

export namespace blog::handlers::operations::tiers::browse::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
    std::string query{};
};

} // namespace
