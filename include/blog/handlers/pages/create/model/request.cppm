export module blog_handlers:pages_create_model;
import std;
export namespace blog::handlers::pages::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
