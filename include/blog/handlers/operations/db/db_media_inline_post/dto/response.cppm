export module blog_operations:db_db_media_inline_post_dto;

import std;

export namespace blog::handlers::operations::db::db_media_inline_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
