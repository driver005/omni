export module blog_operations:members_metafields_read_model;

import std;

export namespace blog::handlers::operations::members::metafields_read::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
