export module blog_handlers:labels_destroy_model;
import std;
export namespace blog::handlers::labels::destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
