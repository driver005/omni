export module blog_handlers:pages_edit_model;
import std;
export namespace blog::handlers::pages::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
