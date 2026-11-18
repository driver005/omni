export module blog_operations:members_bulk_destroy_model;

import std;

export namespace blog::handlers::operations::members::bulk_destroy::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
