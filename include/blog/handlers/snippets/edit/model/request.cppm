export module blog_handlers:snippets_edit_model;
import std;
export namespace blog::handlers::snippets::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
