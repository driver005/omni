export module blog_handlers:db_bulk_destroy_model;
import std;
export namespace blog::handlers::db::bulk_destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
