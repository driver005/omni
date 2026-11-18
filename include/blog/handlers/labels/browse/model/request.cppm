export module blog_handlers:labels_browse_model;
import std;
export namespace blog::handlers::labels::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
