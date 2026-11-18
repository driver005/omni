export module blog_operations:db_bulk_destroy_model;

import std;

export namespace blog::handlers::operations::db::bulk_destroy::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
