export module blog_handlers:config_browse_model;
import std;
export namespace blog::handlers::config::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
