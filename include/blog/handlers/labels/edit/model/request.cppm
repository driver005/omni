export module blog_handlers:labels_edit_model;
import std;
export namespace blog::handlers::labels::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
