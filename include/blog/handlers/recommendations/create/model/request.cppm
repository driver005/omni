export module blog_handlers:recommendations_create_model;
import std;
export namespace blog::handlers::recommendations::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
