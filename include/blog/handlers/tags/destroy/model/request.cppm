export module blog_handlers:tags_destroy_model;
import std;
export namespace blog::handlers::tags::destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
