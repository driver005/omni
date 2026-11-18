export module blog_handlers:mentions_browse_model;
import std;
export namespace blog::handlers::mentions::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
