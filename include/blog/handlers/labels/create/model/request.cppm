export module blog_handlers:labels_create_model;
import std;
export namespace blog::handlers::labels::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
