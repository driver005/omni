export module blog_handlers:webhooks_destroy_model;
import std;
export namespace blog::handlers::webhooks::destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
