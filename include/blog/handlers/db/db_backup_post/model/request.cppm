export module blog_handlers:db_db_backup_post_model;
import std;
export namespace blog::handlers::db::db_backup_post::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
