export module blog_operations:db_db_media_inline_post_model;

import std;

export namespace blog::handlers::operations::db::db_media_inline_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
