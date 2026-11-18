export module blog_handlers:members_bulk_edit_model;
import std;
export namespace blog::handlers::members::bulk_edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
