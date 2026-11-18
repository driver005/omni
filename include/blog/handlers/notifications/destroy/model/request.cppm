export module blog_handlers:notifications_destroy_model;
import std;
export namespace blog::handlers::notifications::destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
