export module blog_handlers:themes_browse_model;
import std;
export namespace blog::handlers::themes::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
