export module blog_handlers:recommendations_destroy_model;
import std;
export namespace blog::handlers::recommendations::destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
