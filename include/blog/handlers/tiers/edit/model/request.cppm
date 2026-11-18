export module blog_handlers:tiers_edit_model;
import std;
export namespace blog::handlers::tiers::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
