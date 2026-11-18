export module blog_operations:db_create_model;

import std;

export namespace blog::handlers::operations::db::create::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
