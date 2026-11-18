export module blog_handlers:recommendations_edit_model;
import std;
export namespace blog::handlers::recommendations::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
