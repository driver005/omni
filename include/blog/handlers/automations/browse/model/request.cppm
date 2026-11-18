export module blog_handlers:automations_browse_model;
import std;
export namespace blog::handlers::automations::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
