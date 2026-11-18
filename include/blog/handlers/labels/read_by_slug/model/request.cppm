export module blog_handlers:labels_read_by_slug_model;
import std;
export namespace blog::handlers::labels::read_by_slug::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
