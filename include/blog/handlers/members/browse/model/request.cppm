export module blog_handlers:members_browse_model;
import std;
export namespace blog::handlers::members::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
