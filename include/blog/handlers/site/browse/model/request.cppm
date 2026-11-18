export module blog_handlers:site_browse_model;
import std;
export namespace blog::handlers::site::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
