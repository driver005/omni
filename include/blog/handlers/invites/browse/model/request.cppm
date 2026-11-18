export module blog_handlers:invites_browse_model;
import std;
export namespace blog::handlers::invites::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
