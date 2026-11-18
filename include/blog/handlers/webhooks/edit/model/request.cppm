export module blog_handlers:webhooks_edit_model;
import std;
export namespace blog::handlers::webhooks::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
