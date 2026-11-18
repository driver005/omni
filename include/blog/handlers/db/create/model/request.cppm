export module blog_handlers:db_create_model;
import std;
export namespace blog::handlers::db::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
