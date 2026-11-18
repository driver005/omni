export module blog_handlers:comments_browse_model;
import std;
export namespace blog::handlers::comments::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
