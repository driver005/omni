export module blog_handlers:settings_browse_model;
import std;
export namespace blog::handlers::settings::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
