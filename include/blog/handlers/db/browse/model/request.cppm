export module blog_handlers:db_browse_model;
import std;
export namespace blog::handlers::db::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
