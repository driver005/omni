export module blog_handlers:tags_create_model;
import std;
export namespace blog::handlers::tags::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
