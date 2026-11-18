export module blog_handlers:snippets_create_model;
import std;
export namespace blog::handlers::snippets::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
