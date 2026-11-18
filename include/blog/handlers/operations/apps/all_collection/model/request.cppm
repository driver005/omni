export module blog_operations:apps_all_collection_model;

import std;

export namespace blog::handlers::operations::apps::all_collection::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
