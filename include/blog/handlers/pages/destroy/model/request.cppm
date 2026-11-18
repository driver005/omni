export module blog_handlers:pages_destroy_model;
import std;
export namespace blog::handlers::pages::destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
