export module blog_handlers:tags_edit_model;
import std;
export namespace blog::handlers::tags::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
