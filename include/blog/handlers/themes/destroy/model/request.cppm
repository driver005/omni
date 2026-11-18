export module blog_handlers:themes_destroy_model;
import std;
export namespace blog::handlers::themes::destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
