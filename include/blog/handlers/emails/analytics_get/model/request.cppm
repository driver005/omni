export module blog_handlers:emails_analytics_get_model;
import std;
export namespace blog::handlers::emails::analytics_get::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
