export module blog_handlers:newsletters_browse_model;
import std;
export namespace blog::handlers::newsletters::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
