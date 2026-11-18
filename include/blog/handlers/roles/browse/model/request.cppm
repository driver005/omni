export module blog_handlers:roles_browse_model;
import std;
export namespace blog::handlers::roles::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
