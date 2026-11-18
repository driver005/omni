export module blog_handlers:users_browse_model;
import std;
export namespace blog::handlers::users::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
