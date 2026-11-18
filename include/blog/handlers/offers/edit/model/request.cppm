export module blog_handlers:offers_edit_model;
import std;
export namespace blog::handlers::offers::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
