export module blog_handlers:snippets_read_by_id_model;
import std;
export namespace blog::handlers::snippets::read_by_id::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
