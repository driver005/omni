export module blog_handlers:newsletters_create_model;
import std;
export namespace blog::handlers::newsletters::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
