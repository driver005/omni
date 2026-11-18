export module blog_handlers:exports_create_model;
import std;
export namespace blog::handlers::exports::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
