export module blog_handlers:authors_browse_model;
import std;
export namespace blog::handlers::authors::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
