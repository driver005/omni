export module blog_handlers:recommendations_browse_model;
import std;
export namespace blog::handlers::recommendations::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
