export module blog_handlers:members_edit_model;
import std;
export namespace blog::handlers::members::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
