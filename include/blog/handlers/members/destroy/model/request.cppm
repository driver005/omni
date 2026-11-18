export module blog_handlers:members_destroy_model;
import std;
export namespace blog::handlers::members::destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
