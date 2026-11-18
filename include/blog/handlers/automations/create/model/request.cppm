export module blog_handlers:automations_create_model;
import std;
export namespace blog::handlers::automations::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
