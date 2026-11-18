export module blog_handlers:users_edit_model;
import std;
export namespace blog::handlers::users::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
