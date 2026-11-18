export module blog_handlers:emails_browse_model;
import std;
export namespace blog::handlers::emails::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
