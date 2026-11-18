export module blog_handlers:notifications_browse_model;
import std;
export namespace blog::handlers::notifications::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
