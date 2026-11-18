export module blog_handlers:integrations_create_model;
import std;
export namespace blog::handlers::integrations::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
