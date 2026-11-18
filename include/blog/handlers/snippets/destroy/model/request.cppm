export module blog_handlers:snippets_destroy_model;
import std;
export namespace blog::handlers::snippets::destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
