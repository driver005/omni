export module blog_operations:db_db_backup_post_model;

import std;

export namespace blog::handlers::operations::db::db_backup_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
