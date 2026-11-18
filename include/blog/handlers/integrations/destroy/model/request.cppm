export module blog_handlers:integrations_destroy_model;
import std;
export namespace blog::handlers::integrations::destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
