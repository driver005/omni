export module blog_handlers:invites_create_model;
import std;
export namespace blog::handlers::invites::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
