export module blog_handlers:comments_edit_model;
import std;
export namespace blog::handlers::comments::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
