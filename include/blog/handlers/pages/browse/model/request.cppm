export module blog_handlers:pages_browse_model;
import std;
export namespace blog::handlers::pages::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
