export module blog_operations:search_index_search_index_authors_get_model;

import std;

export namespace blog::handlers::operations::search_index::search_index_authors_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
