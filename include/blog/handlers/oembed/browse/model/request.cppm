export module blog_handlers:oembed_browse_model;
import std;
export namespace blog::handlers::oembed::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
