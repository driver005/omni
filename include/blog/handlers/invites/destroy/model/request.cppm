export module blog_handlers:invites_destroy_model;
import std;
export namespace blog::handlers::invites::destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
