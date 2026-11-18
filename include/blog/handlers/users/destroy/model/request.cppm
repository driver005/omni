export module blog_handlers:users_destroy_model;
import std;
export namespace blog::handlers::users::destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
