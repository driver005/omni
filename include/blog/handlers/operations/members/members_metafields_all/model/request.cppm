export module blog_operations:members_members_metafields_all_model;

import std;

export namespace blog::handlers::operations::members::members_metafields_all::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
