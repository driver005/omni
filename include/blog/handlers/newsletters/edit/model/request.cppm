export module blog_handlers:newsletters_edit_model;
import std;
export namespace blog::handlers::newsletters::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
