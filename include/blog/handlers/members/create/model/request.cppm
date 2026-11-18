export module blog_handlers:members_create_model;
import std;
export namespace blog::handlers::members::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
