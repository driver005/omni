export module blog_handlers:links_bulk_edit_model;
import std;
export namespace blog::handlers::links::bulk_edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
