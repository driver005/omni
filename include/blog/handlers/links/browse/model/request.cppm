export module blog_handlers:links_browse_model;
import std;
export namespace blog::handlers::links::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
