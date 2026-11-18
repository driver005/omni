export module blog_handlers:offers_browse_model;
import std;
export namespace blog::handlers::offers::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
