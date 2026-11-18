export module blog_handlers:snippets_browse_model;
import std;
export namespace blog::handlers::snippets::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
