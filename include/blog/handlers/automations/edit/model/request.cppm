export module blog_handlers:automations_edit_model;
import std;
export namespace blog::handlers::automations::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
