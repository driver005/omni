export module blog_handlers:integrations_edit_model;
import std;
export namespace blog::handlers::integrations::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
