export module blog_handlers:actions_browse_model;
import std;
export namespace blog::handlers::actions::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
