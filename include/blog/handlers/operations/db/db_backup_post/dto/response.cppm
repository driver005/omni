export module blog_operations:db_db_backup_post_dto;

import std;

export namespace blog::handlers::operations::db::db_backup_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
